/***************************************************************************
 *
 * Multitouch X driver
 * Copyright (C) 2008 Henrik Rydberg <rydberg@euromail.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **************************************************************************/

#include "mtdev.h"
#include "mtbit.h"

/**
 * struct MTSlot - represents the state of an input MT slot
 * @abs: current values of ABS_MT axes for this slot
 */
struct MTSlot {
	int abs[MT_ABS_SIZE];
};

/**
 * struct MTDevState - MT slot parsing
 * @data: array of scratch slot data
 * @used: bitmask of currently used slots
 * @slot: slot currently being modified
 * @lastid: last used tracking id
 */
struct MTDevState {
	struct MTSlot data[DIM_FINGER];
	bitmask_t used;
	bitmask_t slot;
	bitmask_t lastid;
};

/**
 * mtdev_init - init MT device
 * @dev: device to initialize
 * @caps: device capabilities
 */
int mtdev_init(struct MTDev *dev, const struct Capabilities *caps)
{
	memset(dev, 0, sizeof(struct MTDev));
	if (!caps->has_mtdata)
		return -ENODEV;
	if (!caps->has_slot) {
		dev->priv = calloc(1, sizeof(struct MTDevState));
		if (!dev->priv)
			return -ENOMEM;
	}
	return 0;
}

static inline int istouch(const struct MTSlot *data,
			  const struct Capabilities *caps)
{
	return data->abs[BIT_TOUCH_MAJOR] || !caps->has_abs[BIT_TOUCH_MAJOR];
}

/* Dmitry Torokhov's code from kernel/driver/input/input.c */
static int defuzz(int value, int old_val, int fuzz)
{
	if (fuzz) {
		if (value > old_val - fuzz / 2 && value < old_val + fuzz / 2)
			return old_val;

		if (value > old_val - fuzz && value < old_val + fuzz)
			return (old_val * 3 + value) / 4;

		if (value > old_val - fuzz * 2 && value < old_val + fuzz * 2)
			return (old_val + value) / 2;
	}

	return value;
}

/*
 * solve - solve contact matching problem
 * @priv: parsing state
 * @caps: device capabilities
 * @sid: array of current tracking ids
 * @sx: array of current position x
 * @sy: array of current position y
 * @sn: number of current contacts
 * @nid: array of new or matched tracking ids, to be filled
 * @nx: array of new position x
 * @ny: array of new position y
 * @nn: number of new contacts
 * @touch: which of the new contacts to fill
 */
static void solve(struct MTDevState *priv, const struct Capabilities *caps,
		  const int *sid, const int *sx, const int *sy, int sn,
		  int *nid, const int *nx, const int *ny, int nn,
		  bitmask_t touch)
{
	int A[DIM2_FINGER], *row;
	int n2s[DIM_FINGER];
	int id, i, j;

	/* setup distance matrix for contact matching */
	for (j = 0; j < sn; j++) {
		row = A + nn * j;
		for (i = 0; i < nn; i++)
			row[i] = dist2(nx[i] - sx[j], ny[i] - sy[j]);
	}

	match_fingers(n2s, A, nn, sn);

	/* update matched contacts and create new ones */
	foreach_bit(i, touch) {
		j = n2s[i];
		id = j >= 0 ? sid[j] : caps->nullid;
		while (id == caps->nullid)
			id = ++priv->lastid;
		nid[i] = id;
	}
}

/*
 * assign_tracking_id - assign tracking ids to all contacts
 * @priv: parsing state
 * @caps: device capabilities
 * @data: array of all present contacts, to be filled
 * @prop: array of all set contacts properties
 * @size: number of contacts in array
 * @touch: which of the contacts are actual touches
 */
static void assign_tracking_id(struct MTDevState *priv,
			       const struct Capabilities *caps,
			       struct MTSlot *data, bitmask_t *prop,
			       int size, bitmask_t touch)
{
	int sid[DIM_FINGER], sx[DIM_FINGER], sy[DIM_FINGER], sn = 0;
	int nid[DIM_FINGER], nx[DIM_FINGER], ny[DIM_FINGER], i;
	foreach_bit(i, priv->used) {
		sid[sn] = priv->data[i].abs[BIT_TRACKING_ID];
		sx[sn] = priv->data[i].abs[BIT_POSITION_X];
		sy[sn] = priv->data[i].abs[BIT_POSITION_Y];
		sn++;
	}
	for (i = 0; i < size; i++) {
		nx[i] = data[i].abs[BIT_POSITION_X];
		ny[i] = data[i].abs[BIT_POSITION_Y];
	}
	solve(priv, caps, sid, sx, sy, sn, nid, nx, ny, size, touch);
	for (i = 0; i < size; i++) {
		data[i].abs[BIT_TRACKING_ID] =
			GETBIT(touch, i) ? nid[i] : caps->nullid;
		prop[i] |= BITMASK(BIT_TRACKING_ID);
	}
}

/*
 * process_typeA - consume MT events and update parsing state
 * @dev: MT device
 * @data: array of all present contacts, to be filled
 * @prop: array of all set contacts properties, to be filled
 *
 * This function is called when a SYN_REPORT is seen, right before
 * that event is pushed to the queue.
 *
 * Returns -1 if the packet is not MT related and should not affect
 * the current parsing state.
 */
static int process_typeA(struct MTDev *dev,
			 struct MTSlot *data, bitmask_t *prop)
{
	struct input_event ev;
	int consumed, mtcode;
	int mtcnt = 0, size = 0;
	prop[size] = 0;
	while (!evbuf_empty(&dev->inbuf)) {
		evbuf_pop(&dev->inbuf, &ev);
		consumed = 0;
		switch (ev.type) {
		case EV_SYN:
			switch (ev.code) {
			case SYN_MT_REPORT:
				if (size < DIM_FINGER &&
				    GETBIT(prop[size], BIT_POSITION_X) &&
				    GETBIT(prop[size], BIT_POSITION_Y))
					size++;
				if (size < DIM_FINGER)
					prop[size] = 0;
				mtcnt++;
				consumed = 1;
				break;
			}
			break;
		case EV_KEY:
			switch (ev.code) {
			case BTN_TOUCH:
				mtcnt++;
				break;
			}
			break;
		case EV_ABS:
			if (size < DIM_FINGER && has_abs2mt(ev.code)) {
				mtcode = abs2mt(ev.code);
				data[size].abs[mtcode] = ev.value;
				prop[size] |= BITMASK(mtcode);
				mtcnt++;
				consumed = 1;
			}
			break;
		}
		if (!consumed)
			evbuf_push(&dev->outbuf, &ev);
	}
	return mtcnt ? size : -1;
}

/*
 * process_typeB - propagate events without parsing
 * @dev: MT device
 *
 * This function is called when a SYN_REPORT is seen, right before
 * that event is pushed to the queue.
 */
static void process_typeB(struct MTDev *dev)
{
	struct input_event ev;
	while (!evbuf_empty(&dev->inbuf)) {
		evbuf_pop(&dev->inbuf, &ev);
		evbuf_push(&dev->outbuf, &ev);
	}
}

/*
 * filter_data - apply input filtering on new incoming data
 * @priv: parsing state
 * @caps: device capabilities
 * @data: the incoming data to filter
 * @prop: the properties to filter
 * @slot: the slot the data refers to
 */
static void filter_data(const struct MTDevState *priv,
			const struct Capabilities *caps,
			struct MTSlot *data, bitmask_t prop,
			int slot)
{
	int i;
	foreach_bit(i, prop) {
		int fuzz = caps->abs[i].fuzz;
		int oldval = priv->data[slot].abs[i];
		data->abs[i] = defuzz(data->abs[i], oldval, fuzz);
	}
}

/*
 * push_slot_changes - propagate state changes
 * @dev: MT device
 * @data: the incoming data to propagate
 * @prop: the properties to propagate
 * @slot: the slot the data refers to
 * @syn: reference to the SYN_REPORT event
 */
static void push_slot_changes(struct MTDev *dev,
			      const struct MTSlot *data, bitmask_t prop,
			      int slot, const struct input_event *syn)
{
	struct MTDevState *priv = dev->priv;
	struct input_event ev;
	int i, count = 0;
	foreach_bit(i, prop)
		if (priv->data[slot].abs[i] != data->abs[i])
			count++;
	if (!count)
		return;
	ev.time = syn->time;
	ev.type = EV_ABS;
	ev.code = ABS_MT_SLOT;
	ev.value = slot;
	if (priv->slot != ev.value) {
		evbuf_push(&dev->outbuf, &ev);
		priv->slot = ev.value;
	}
	foreach_bit(i, prop) {
		ev.code = mt2abs(i);
		ev.value = data->abs[i];
		if (priv->data[slot].abs[i] != ev.value) {
			evbuf_push(&dev->outbuf, &ev);
			priv->data[slot].abs[i] = ev.value;
		}
	}
}

/*
 * apply_typeA_changes - parse and propagate state changes
 * @dev: MT device
 * @caps: device capabilities
 * @data: array of data to apply
 * @prop: array of properties to apply
 * @size: number of contacts in array
 * @syn: reference to the SYN_REPORT event
 */
static void apply_typeA_changes(struct MTDev *dev,
				const struct Capabilities *caps,
				struct MTSlot *data, const bitmask_t *prop,
				int size, const struct input_event *syn)
{
	struct MTDevState *priv = dev->priv;
	bitmask_t unused = ~priv->used;
	bitmask_t used = 0;
	int i, slot, id;
	for (i = 0; i < size; i++) {
		id = data[i].abs[BIT_TRACKING_ID];
		foreach_bit(slot, priv->used) {
			if (priv->data[slot].abs[BIT_TRACKING_ID] != id)
				continue;
			filter_data(priv, caps, &data[i], prop[i], slot);
			push_slot_changes(dev, &data[i], prop[i], slot, syn);
			SETBIT(used, slot);
			id = caps->nullid;
			break;
		}
		if (id != caps->nullid) {
			slot = firstbit(unused);
			push_slot_changes(dev, &data[i], prop[i], slot, syn);
			SETBIT(used, slot);
			CLEARBIT(unused, slot);
		}
	}

	/* clear unused slots and update slot usage */
	foreach_bit(slot, priv->used & ~used) {
		struct MTSlot tdata = priv->data[slot];
		bitmask_t tprop = BITMASK(BIT_TRACKING_ID);
		tdata.abs[BIT_TRACKING_ID] = caps->nullid;
		push_slot_changes(dev, &tdata, tprop, slot, syn);
	}
	priv->used = used;
}

/*
 * convert_A_to_B - propagate a type A packet as a type B packet
 * @dev: MT device
 * @caps: device capabilities
 * @syn: reference to the SYN_REPORT event
 */
static void convert_A_to_B(struct MTDev *dev,
			   const struct Capabilities *caps,
			   const struct input_event *syn)
{
	struct MTSlot data[DIM_FINGER];
	bitmask_t prop[DIM_FINGER];
	int size = process_typeA(dev, data, prop);
	if (size < 0)
		return;
	if (!caps->has_abs[BIT_TRACKING_ID]) {
		bitmask_t touch = 0;
		int i;
		for (i = 0; i < size; i++)
			MODBIT(touch, i, istouch(&data[i], caps));
		assign_tracking_id(dev->priv, caps, data, prop, size, touch);
	}
	apply_typeA_changes(dev, caps, data, prop, size, syn);
}

/**
 * mtdev_push - insert event into MT device
 * @dev: MT device
 * @caps: device capabilities
 * @syn: reference to the SYN_REPORT event
 */
void mtdev_push(struct MTDev *dev,
		const struct Capabilities *caps,
		const struct input_event *ev)
{
	if (ev->type == EV_SYN && ev->code == SYN_REPORT) {
		bitmask_t head = dev->outbuf.head;
		if (dev->priv)
			convert_A_to_B(dev, caps, ev);
		else
			process_typeB(dev);
		if (dev->outbuf.head != head)
			evbuf_push(&dev->outbuf, ev);
	} else {
		evbuf_push(&dev->inbuf, ev);
	}
}

/**
 * mtdev_destroy - destroy MT device
 * @dev: MT device
 */
void mtdev_destroy(struct MTDev *dev)
{
	free(dev->priv);
	memset(dev, 0, sizeof(struct MTDev));
}
