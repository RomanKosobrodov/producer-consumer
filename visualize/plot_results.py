import argparse
import json
import numpy as np
import matplotlib.pyplot as plt


def match(r, implementation=None, num_cycles=None, block_size=None, num_blocks=None, num_readers=None):
    combined = list()
    for x in r:
        if implementation is not None and x["implementation"] != implementation:
            continue
        if num_cycles is not None and int(x["num_cycles"]) != num_cycles:
            continue
        if block_size is not None and int(x["block_size"]) != block_size:
            continue
        if num_blocks is not None and int(x["num_blocks"]) != num_blocks:
            continue
        if num_readers is not None and int(x["num_readers"]) != num_readers:
            continue
        combined.append(x)
    return combined


def plot_performance(baseline, seqlock, shared_mutex, mutex, zmq=None,
                     write_filename="plots/write.png",
                     read_filename="plots/read.png",
                     dpi=300):

    block_size = [x["block_size"] for x in baseline]
    baseline_writer = [x["writer"] for x in baseline]
    seqlock_writer = [x["writer"] for x in seqlock]
    shared_writer = [x["writer"] for x in shared_mutex]
    mutex_writer = [x["writer"] for x in mutex]
    if zmq is not None:
        zmq_writer = [x["writer"] for x in zmq]

    baseline_readers = np.array([x["readers"] for x in baseline])
    seqlock_readers = np.array([x["readers"] for x in seqlock])
    shared_readers = np.array([x["readers"] for x in shared_mutex])
    mutex_readers = np.array([x["readers"] for x in mutex])
    if zmq is not None:
        zmq_readers = np.array([x["readers"] for x in zmq])

    plt.figure("Write performance")
    plt.semilogy(block_size, baseline_writer, "k.-", label="Baseline")
    plt.semilogy(block_size, seqlock_writer, "r.-", label="SeqLock")
    plt.semilogy(block_size, shared_writer, "g.-", label="Shared mutex")
    plt.semilogy(block_size, mutex_writer, "b.-", label="Mutex")
    if zmq is not None:
        plt.semilogy(block_size, zmq_writer, "m.-", label="ZMQ")

    plt.xlabel("block size, double precision samples")
    plt.ylabel("time per operation, ns")
    plt.legend(frameon=False)
    plt.savefig(write_filename, dpi=dpi)

    plt.figure("Read performance")
    for k in range(mutex_readers.shape[1]):
        if k == 0:
            plt.semilogy(block_size, baseline_readers[:, k], "k.-", label="Baseline")
            plt.semilogy(block_size, seqlock_readers[:, k], "r.-", label="SeqLock")
            plt.semilogy(block_size, shared_readers[:, k], "g.-", label="Shared mutex")
            plt.semilogy(block_size, mutex_readers[:, k], "b.-", label="Mutex")
            if zmq is not None:
                plt.semilogy(block_size, zmq_readers[:, k], "m.-", label="ZMQ")
        else:
            plt.semilogy(block_size, baseline_readers[:, k], "k.-")
            plt.semilogy(block_size, seqlock_readers[:, k], "r.-")
            plt.semilogy(block_size, shared_readers[:, k], "g.-")
            plt.semilogy(block_size, mutex_readers[:, k], "b.-")
            if zmq is not None:
                plt.semilogy(block_size, zmq_readers[:, k], "m.-")

    plt.xlabel("block size, double precision samples")
    plt.ylabel("time per operation, ns")
    plt.legend(frameon=False)
    plt.savefig(read_filename, dpi=dpi)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot benchmark results")
    parser.add_argument("data",
                        help="JSON file with benchmark results",
                        type=str)
    parser.add_argument("--zmq",
                        help="(optional) include ZeroMQ results. Disabled by default",
                        action="store_true")
    args = parser.parse_args()

    with open(args.data, "r") as fp:
        content = json.load(fp)
        results = content["results"]

    num_readers = 3
    baseline_dataset = match(results, implementation="Memcpy", num_readers=num_readers)
    seqlock_dataset = match(results, implementation="SeqLock", num_readers=num_readers)
    shared_mutex_dataset = match(results, implementation="Shared mutex", num_readers=num_readers)
    mutex_dataset = match(results, implementation="Mutex", num_readers=num_readers)
    if args.zmq:
        zmq_dataset = match(results, implementation="ZMQ", num_readers=num_readers)
    else:
        zmq_dataset = None

    plot_performance(baseline_dataset, seqlock_dataset, shared_mutex_dataset, mutex_dataset, zmq_dataset)

    plt.show()