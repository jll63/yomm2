import contextlib
import json
import logging
import os
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, NamedTuple, Optional, get_type_hints

logger = logging.getLogger(__name__)

axes = dict(
    dispatch=(
        "virtual_function",
        "basic_policy",
        "compact_map_policy",
        "direct_intrusive",
        "indirect_intrusive",
        "direct_virtual_ptr",
        "indirect_virtual_ptr",
    ),
    arity=("arity_1", "arity_2"),
    inheritance=("ordinary_base", "virtual_base"),
)


@dataclass
class Distribution:
    lo: float = 0
    hi: float = 0
    median: float = 0
    stddev: float = 0


@dataclass
class Benchmark:
    tags: list[str]
    mean: float = 0
    median: float = 0
    stddev: float = 0
    cv: float = 0
    base: Optional["Benchmark"] = None

    @property
    def dispatch(self) -> str:
        return self.tags[0]


class Cache(NamedTuple):
    type: str
    level: int
    size: int
    num_sharing: int


class YOMM2Context(NamedTuple):
    build_type: str
    compiler: str
    compiler_version: str
    hierarchies: int
    objects: int
    # ti_ptrs: Distribution
    # obj_ptrs: Distribution


class Context(NamedTuple):
    date: str
    host_name: str
    executable: str
    num_cpus: int
    mhz_per_cpu: int
    cpu_scaling_enabled: bool
    caches: list[Cache]
    load_avg: list[float]
    library_build_type: str
    yomm2: YOMM2Context

    @classmethod
    def parse(cls, data):
        data = {**data}
        types = get_type_hints(YOMM2Context)
        yomm2 = YOMM2Context(
            **{
                k: types[k](data.pop(pk))
                for pk in list(data.keys())
                if pk.startswith("yomm2_")
                for k in (pk[len("yomm2_") :],)
            }
        )

        return cls(
            **{
                **data,
                "caches": [Cache(**cache_data) for cache_data in data["caches"]],
                "yomm2": yomm2,
            },
        )

    def write_md(self, md):
        md.new_header(level=2, title="Context", add_table_of_contents=False)
        md.new_line(
            "{hierarchies} hierarchies of {objects} objects".format(
                **self.yomm2._asdict()
            ),
        )
        md.new_line(
            "Compiler: {compiler} {compiler_version}".format(**self.yomm2._asdict())
        )
        md.new_line("Build type: {build_type}".format(**self.yomm2._asdict()))
        md.new_line("Load average: {:.2f}, {:.2f}, {:.2f}".format(*self.load_avg))
        md.new_line("Run on `{host_name}` on {date}".format(**self._asdict()))
        md.new_line("{num_cpus} X {mhz_per_cpu} MHZ CPUs".format(**self._asdict()))
        md.new_line("CPU caches:  ")
        for cache in self.caches:
            md.new_line(
                "&nbsp;&nbsp;L{level} {type} {size} KiB (x{num_sharing})".format(
                    **cache._asdict()
                )
            )


class Benchmarks(NamedTuple):
    data: dict
    context: Context
    index: dict[str, Benchmark]

    @classmethod
    def parse(cls, data: dict):
        index: dict[str, Benchmark] = {}
        baseline: Benchmark | None = None

        for benchmark_data in data["benchmarks"]:
            run_name = benchmark_data["run_name"]
            tags = run_name.split("-")
            benchmark = index.setdefault(run_name, Benchmark(tags))
            setattr(
                benchmark, benchmark_data["aggregate_name"], benchmark_data["cpu_time"]
            )

        baseline = index.pop("baseline")

        for benchmark in index.values():
            benchmark.mean -= baseline.mean
            benchmark.median -= baseline.median
            if benchmark.mean < 0:
                breakpoint()
            if benchmark.dispatch != "virtual_function":
                benchmark.base = index[
                    "-".join(["virtual_function", *benchmark.tags[1:]])
                ]

        return cls(data, Context.parse(data["context"]), index)

    @classmethod
    def run(cls, exe: Path, *benchmark_args, objects: int = None):
        benchmark_args = (
            "--benchmark_format=json",
            "--benchmark_repetitions=10",
            "--benchmark_report_aggregates_only=true",
            "--benchmark_enable_random_interleaving=true",
            *benchmark_args,
        )

        env = os.environ.copy()
        if objects is not None:
            env["YOMM2_BENCHMARKS_OBJECTS"] = str(objects)
            log_prefix = f"YOMM2_BENCHMARKS_OBJECTS={objects} "
        else:
            log_prefix = ""

        logger.info("running %s%s", log_prefix, " ".join([str(exe), *benchmark_args]))
        with subprocess.Popen(
            [exe, *benchmark_args],
            stdout=subprocess.PIPE,
            env=env,
        ) as bm:
            return Benchmarks.parse(json.load(bm.stdout))

    def get(self, *tags: str) -> Benchmark:
        return self.index["-".join(map(str, tags))]

    @property
    def all(self) -> Iterable[Benchmark]:
        return self.index.values()


PARAMETERS_HEADER = Path("tests/benchmarks_parameters.hpp")


def build(build_dir: Path, nh: int = None) -> Path:
    exe = build_dir.joinpath("tests", "benchmarks")
    with contextlib.ExitStack() as scope:
        if nh is not None:
            logger.info("compiling %s for %d hierarchies", exe, nh)
            with open(PARAMETERS_HEADER) as hpp:
                previous_params = hpp.read()

            def write_params(text: str):
                with open(PARAMETERS_HEADER, "w") as hpp:
                    hpp.write(text)

            scope.callback(write_params, previous_params)
            write_params(f"#define YOMM2_BENCHMARK_HIERARCHIES {nh}\n")

        cmd = f"make -C {build_dir} benchmarks"
        subprocess.check_call(cmd.split(), stdout=subprocess.DEVNULL)
    return exe
