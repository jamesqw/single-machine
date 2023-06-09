# Confluo

[![Build Status](https://amplab.cs.berkeley.edu/jenkins/job/confluo/badge/icon)](https://amplab.cs.berkeley.edu/jenkins/job/confluo/)
[![License](http://img.shields.io/:license-Apache%202-red.svg)](LICENSE)

Confluo is a system for real-time monitoring and analysis of data, that supports:
* high-throughput concurrent writes of millions of data points from multiple data streams;
* online queries at millisecond timescale; and 
* ad-hoc queries using minimal CPU resources.

Please find detailed documentation [here](https://ucbrise.github.io/confluo/).

## Installation

Required dependencies:

* MacOS X or Unix-based OS; Windows is not yet supported.
* C++ compiler that supports C++11 standard (e.g., GCC 5.3 or later)
* CMake 3.2 or later
* Boost 1.58 or later

For python client, you will additionally require:

* Python 2.7 or later
* Python Packages: setuptools, six 1.7.2 or later

For java client, you will additionally require:

* Java JDK 1.7 or later
* ant 1.6.2 or later

### Source Build

To download and install Confluo, use the following commands:

```bash
git clone https://github.com/ucbrise/confluo.git
cd confluo
mkdir build
cd build
cmake ..
make -j && make test && make install
```

## Using Confluo

While Confluo supports multiple execution modes, the simplest way to get 
started is to start Confluo as a server daemon and query it using one of
its client APIs.

To start the server daemon, run:

```bash
confluod --address=127.0.0.1 --port=9090
```

Here's some sample usage of the Python API:

```python
import sys
from confluo.rpc.client import RpcClient
from confluo.rpc.storage import StorageMode

# Connect to the server
client = RpcClient("127.0.0.1", 9090)

# Create an Atomic MultiLog with given schema for a performance log
schema = """{
  timestamp: ULONG,
  op_latency_ms: DOUBLE,
  cpu_util: DOUBLE,
  mem_avail: DOUBLE,
  log_msg: STRING(100)
}"""
storage_mode = StorageMode.IN_MEMORY
client.create_atomic_multilog("perf_log", schema, storage_mode)

# Add an index
client.add_index("op_latency_ms")

# Add a filter
client.add_filter("low_resources", "cpu_util>0.8 || mem_avail<0.1")

# Add an aggregate
client.add_aggregate("max_latency_ms", "low_resources", "MAX(op_latency_ms)")

# Install a trigger
client.install_trigger("high_latency_trigger", "max_latency_ms > 1000")

# Load some data
off1 = client.append([100.0, 0.5, 0.9,  "INFO: Launched 1 tasks"])
off2 = client.append([500.0, 0.9, 0.05, "WARN: Server {2} down"])
off3 = client.append([1001.0, 0.9, 0.03, "WARN: Server {2, 4, 5} down"])

# Read the written data
record1 = client.read(off1)
record2 = client.read(off2)
record3 = client.read(off3)

# Query using indexes
record_stream = client.execute_filter("cpu_util>0.5 || mem_avail<0.5")
for r in record_stream:
  print r

# Query using filters
record_stream = client.query_filter("low_resources", 0, sys.maxsize)
for r in record_stream:
  print r

# Query an aggregate
print client.get_aggregate("max_latency_ms", 0, sys.maxsize)

# Query alerts generated by a trigger
alert_stream = client.get_alerts(0, sys.maxsize, "high_latency_trigger")
for a in alert_stream:
  print a
```

## Contributing

Please create a GitHub issue to file a bug or request a feature. We welcome pull-requests, but request that you review the [pull-request process](CONTRIBUTING.md) before submitting one.


