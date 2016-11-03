import os
import math
from jinja2 import Environment, FileSystemLoader

from .build import Builder
from .executor import Executor


def render_template(template_file, context):
    path = os.path.dirname(os.path.abspath(__file__))
    environment = Environment(
        autoescape=False,
        loader=FileSystemLoader(os.path.join(path, 'templates')),
        trim_blocks=False)

    return environment.get_template(template_file).render(context)


def generate_source(template_file, source_file, **context):
    code = render_template(template_file, context)

    # Save source file
    with open(source_file, 'w') as f:
        f.write(code)


class Strategy(object):
    def __init__(self, configuration, device_configuration, eviction_counter,
                 number_of_accesses_in_loop, different_addresses_in_loop,
                 step_size, mirroring):
        self.device_configuration = device_configuration
        self.eviction_counter = eviction_counter
        self.number_of_accesses_in_loop = number_of_accesses_in_loop
        self.different_addresses_in_loop = different_addresses_in_loop
        self.step_size = step_size
        self.mirroring = mirroring
        self.number_of_addresses = self.eviction_counter + self.different_addresses_in_loop - 1
        self.configuration = configuration
        self.builder = Builder(self.configuration, self)
        self.executor = Executor(self.configuration, self, self.builder)

    def build(self, force):
        return self.builder.build(force)

    def run(self, number_of_measurements, force):
        return self.executor.run(number_of_measurements, force)

    def get_logfile_name(self):
        device_codename = self.device_configuration['device']['codename']
        log_dir = os.path.join(self.configuration['logs']['directory'], device_codename)
        local_logfile = os.path.join(log_dir, self.get_name() + ".log")
        return local_logfile

    def save_strategy_to_file(self, source_file):
        generate_source(
            'strategy.jinja2',
            source_file,
            number_of_sets=self.device_configuration['cache']['number-of-sets'],
            line_length=self.device_configuration['cache']['line-length'],
            line_length_log2=int(math.log(
                self.device_configuration['cache']['line-length'],
                2)),
            eviction_counter=self.eviction_counter,
            number_of_accesses_in_loop=self.number_of_accesses_in_loop,
            different_addresses_in_loop=self.different_addresses_in_loop,
            step_size=self.step_size)

    def get_name(self):
        name = "%d-%d-%d-%d-%s" % (self.number_of_addresses,
                                self.number_of_accesses_in_loop,
                                self.different_addresses_in_loop,
                                self.step_size,
                                "M" if self.mirroring else "m")
        return name
