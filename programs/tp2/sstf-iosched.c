// SSTF IO Scheduler For Kernel 4.13.9

#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

// SSTF data structure
struct sstf_data {
	struct list_head queue;
	sector_t prev_sector;
};

static void sstf_merged_requests(struct request_queue *q, struct request *rq, struct request *next) {
	list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force) {
	struct sstf_data *nd = q->elevator->elevator_data;

	struct request *rq, *next_rq = NULL;
	sector_t cur_sector = nd->prev_sector;
	sector_t distance = ULONG_MAX;
	char direction;

	// Find the request with the shortest seek time
	list_for_each_entry(rq, &nd->queue, queuelist) {
		sector_t rq_sector = blk_rq_pos(rq);
		sector_t seek_distance = abs(cur_sector - rq_sector);

		if (seek_distance < distance) {
			distance = seek_distance;
			next_rq = rq;
		}
	}

	if (next_rq) {
		list_del_init(&next_rq->queuelist);
		elv_dispatch_sort(q, next_rq);
		nd->prev_sector = blk_rq_pos(next_rq);

		direction = (blk_rq_pos(next_rq) > cur_sector) ? 'R' : 'L';
		printk(KERN_EMERG "[SSTF] dsp %c %llu\n", direction, blk_rq_pos(next_rq));

		return 1;
	}
	return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq) {
	
	struct sstf_data *nd = q->elevator->elevator_data;
	
	sector_t rq_sector = blk_rq_pos(rq);
	sector_t prev_sector = nd->prev_sector;
	char direction = (rq_sector > prev_sector) ? 'R' : 'L';

	// Add the request to the queue based on its position
	struct request *pos, *tmp;
	list_for_each_entry_safe(pos, tmp, &nd->queue, queuelist) {
		sector_t pos_sector = blk_rq_pos(pos);
		if ((rq_sector > prev_sector && pos_sector > rq_sector) || (rq_sector < prev_sector && pos_sector < rq_sector))
			break;
	}

	list_add_tail(&rq->queuelist, &pos->queuelist);
	nd->prev_sector = rq_sector;

	printk(KERN_EMERG "[SSTF] add %c %llu\n", direction, blk_rq_pos(rq));
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e) {
	
	struct sstf_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);
	nd->prev_sector = 0; // Initialize previous sector to 0

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);

	return 0;
}

static void sstf_exit_queue(struct elevator_queue *e) {
	
	struct sstf_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_sstf = {
	.ops.sq = {
		.elevator_merge_req_fn	= sstf_merged_requests,
		.elevator_dispatch_fn	= sstf_dispatch,
		.elevator_add_req_fn	= sstf_add_request,
		.elevator_init_fn		= sstf_init_queue,
		.elevator_exit_fn		= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void) {
	return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void) {
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Miguel Xavier");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
