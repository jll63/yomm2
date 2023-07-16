



## Context
  
100 hierarchies of 1000 objects  
Compiler: clang 14.0.0  
Build type: release  
Load average: 0.23, 0.18, 0.34  
Run on `euclid` on 2023-04-16T10:31:13-04:00  
16 X 3300 MHZ CPUs  
CPU caches:    
&nbsp;&nbsp;L1 Data 32768 KiB (x2)  
&nbsp;&nbsp;L1 Instruction 32768 KiB (x2)  
&nbsp;&nbsp;L2 Unified 524288 KiB (x2)  
&nbsp;&nbsp;L3 Unified 4194304 KiB (x8)  
command line: dev/bm2md --prefix docs/benchmarks/3vec --log-level info  
max cv =   1.5%
## arity 1, ordinary base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function|  66.5|1.00|
|hash factors in globals|  95.6| 1.44|
|hash factors in method|  95.5| 1.43|
|direct intrusive|  71.6| 1.08|
|indirect intrusive|  75.9| 1.14|
  
max cv =   1.5%
## arity 2, ordinary base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function| 106.3|1.00|
|hash factors in globals| 140.2| 1.32|
|hash factors in method| 139.9| 1.32|
|direct intrusive| 121.9| 1.15|
|indirect intrusive| 126.4| 1.19|
  
max cv =   0.2%
## arity 1, virtual base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function|  89.2|1.00|
|hash factors in globals| 186.6| 2.09|
|hash factors in method| 187.6| 2.10|
|direct intrusive| 173.5| 1.94|
|indirect intrusive| 180.7| 2.03|
  
max cv =   1.3%
## arity 2, virtual base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function| 123.6|1.00|
|hash factors in globals| 272.4| 2.20|
|hash factors in method| 265.1| 2.15|
|direct intrusive| 250.1| 2.02|
|indirect intrusive| 252.5| 2.04|
  
max cv =   0.5%