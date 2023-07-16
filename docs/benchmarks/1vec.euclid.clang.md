



## Context
  
100 hierarchies of 1000 objects  
Compiler: clang 14.0.0  
Build type: release  
Load average: 0.93, 1.18, 0.99  
Run on `euclid` on 2023-04-16T10:43:11-04:00  
16 X 3300 MHZ CPUs  
CPU caches:    
&nbsp;&nbsp;L1 Data 32768 KiB (x2)  
&nbsp;&nbsp;L1 Instruction 32768 KiB (x2)  
&nbsp;&nbsp;L2 Unified 524288 KiB (x2)  
&nbsp;&nbsp;L3 Unified 4194304 KiB (x8)  
command line: dev/bm2md --prefix docs/benchmarks/1vec --log-level info  
max cv =   1.5%
## arity 1, ordinary base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function|  66.3|1.00|
|hash factors in globals|  92.5| 1.40|
|hash factors in method|  93.4| 1.41|
|direct intrusive|  70.1| 1.06|
|indirect intrusive|  75.4| 1.14|
  
max cv =   1.5%
## arity 2, ordinary base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function| 106.2|1.00|
|hash factors in globals| 138.5| 1.30|
|hash factors in method| 137.4| 1.29|
|direct intrusive| 121.3| 1.14|
|indirect intrusive| 125.6| 1.18|
  
max cv =   0.7%
## arity 1, virtual base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function|  83.3|1.00|
|hash factors in globals| 180.8| 2.17|
|hash factors in method| 181.1| 2.17|
|direct intrusive| 169.8| 2.04|
|indirect intrusive| 179.0| 2.15|
  
max cv =   0.4%
## arity 2, virtual base

|dispatch|avg|ratio|
| ---: | ---: | ---: |
|virtual function| 121.1|1.00|
|hash factors in globals| 260.3| 2.15|
|hash factors in method| 258.4| 2.13|
|direct intrusive| 242.5| 2.00|
|indirect intrusive| 249.3| 2.06|
  
max cv =   1.2%