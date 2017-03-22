import sys

with open(sys.argv[1], "r") as fh:
    while True:
        size = fh.readline().strip()
        if len(size) == 0:
            break
        k = fh.readline().strip()
        times = fh.readline().split(",")
        times = map(lambda t : float(t), times)
        mean_time = sum(times) / float(len(times))

        print ",".join([size, k, str(mean_time)])

