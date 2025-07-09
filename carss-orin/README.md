# MODIFICATION

* CARSS schedules tasks as FIFO when there is no slack time.
* In the python wrapper (`tag_layer.excl_tag_fn`), it sets no slack flag as `true` which makes ignoring priority.
* Therefore, it is modified as always sets no slack flag to be `true`.

