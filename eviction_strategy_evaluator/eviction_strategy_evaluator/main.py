import click
import logging
import os
import pandas as pd
import sqlite3
from . import log

logger = log.setup_custom_logger('default')

from .strategy import Strategy
from .build import Builder
from .executor import Executor
from .evaluate import evaluate_strategy_logfile

from eviction_strategy_evaluator.config import parse_device_configuration
from eviction_strategy_evaluator.config import parse_configuration

def run_strategy(ctx, number_of_measurements, eviction_counter, number_of_accesses_in_loop,
                 different_addresses_in_loop, step_size, mirroring):
    """Builds and runs an eviction strategy"""
    configuration = ctx.obj['configuration']
    device_configuration = ctx.obj['device-configuration']

    strategy = Strategy(configuration, device_configuration,
                        eviction_counter,
                        number_of_accesses_in_loop,
                        different_addresses_in_loop,
                        step_size,
                        mirroring)

    logger.info("Evaluating %s", strategy.get_name())

    strategy.build(ctx.obj['force'])
    strategy.run(number_of_measurements, ctx.obj['force'])

    return strategy

@click.command('run_strategy')
@click.option('-n', '--number-of-measurements', type=int, required=False, default=10000)
@click.option('-e', '--eviction-counter', type=int, required=True)
@click.option('-a', '--number-of-accesses-in-loop', type=int, required=True)
@click.option('-d', '--different-addresses-in-loop', type=int, required=True)
@click.option('-s', '--step-size', type=int, required=False, default=1)
@click.option('-m', '--mirroring', type=bool, required=False, default=False)
@click.pass_context
def cmd_run_strategy(ctx, number_of_measurements, eviction_counter, number_of_accesses_in_loop,
                 different_addresses_in_loop, step_size, mirroring):
    device_configuration = ctx.obj['device-configuration']

    strategy = run_strategy(ctx, number_of_measurements, eviction_counter, number_of_accesses_in_loop,
                 different_addresses_in_loop, step_size, mirroring)

    logfile = strategy.get_logfile_name()

    # read log file
    result = evaluate_strategy_logfile(logfile, device_configuration, device_configuration['device']['threshold'])
    if result:
        logger.info("Eviction rate: %f%%", result['rate'])
        logger.info("Average runtime: %f", result['average_runtime'])

@click.command('run_strategies')
@click.option('-n', '--number-of-measurements', type=int, required=False, default=10000)
@click.option('-e', '--max-eviction-counter', type=int, required=True)
@click.option('-a', '--max-number-of-accesses-in-loop', type=int, required=True)
@click.option('-d', '--max-different-addresses-in-loop', type=int, required=True)
@click.option('-s', '--max-step-size', type=int, required=False, default=1)
@click.option('-m', '--with-mirroring', type=bool, required=False, default=False)
@click.pass_context
def cmd_run_strategies(ctx, number_of_measurements, max_eviction_counter, max_number_of_accesses_in_loop,
                 max_different_addresses_in_loop, max_step_size, with_mirroring):
    # Generate all strategies and test them
    for a_i in range(max_number_of_accesses_in_loop, 0, -1):
        for d_i in range(max_different_addresses_in_loop, 0, -1):
            for s_i in range(max_step_size, 0, -1):
                if d_i < s_i:
                    continue

                for e_i in range(max_eviction_counter, 0, -1):
                    number_of_addresses = e_i + d_i - 1
                    if (number_of_addresses >= d_i):
                        run_strategy(ctx, number_of_measurements, e_i, a_i, d_i, s_i, False)

                        if with_mirroring is True:
                            run_strategy(ctx, number_of_measurements, e_i, a_i, d_i, s_i, True)

@click.command('evaluate_strategy')
@click.argument('logfile', type=click.Path(exists=True), required=True)
@click.option('-t', '--threshold', type=int, required=True)
@click.pass_context
def cmd_evaluate_strategy(ctx, logfile, threshold):
    device_configuration = ctx.obj['device-configuration']

    # read log file
    result = evaluate_strategy_logfile(logfile, device_configuration, threshold)

    logger.info("Eviction rate: %f%%", result['rate'])
    logger.info("Average runtime: %f", result['average_runtime'])

@click.command('evaluate_strategies')
@click.argument('logfile-directory', type=click.Path(exists=True), required=True)
@click.option('-t', '--threshold', type=int, required=True)
@click.pass_context
def cmd_evaluate_strategies(ctx, logfile_directory, threshold):
    device_configuration = ctx.obj['device-configuration']

    results = []
    for f in sorted(os.listdir(logfile_directory)):
        if not f.endswith(".log"):
            continue

        logfile = os.path.join(logfile_directory, f)

        # read log file
        result = evaluate_strategy_logfile(logfile, device_configuration, threshold)
        if result is not None:
            results.append(result['raw'])

    df = pd.DataFrame(results, columns=['Strategy', 'Number of addresses', 'Number of accesses in loop', 'Different addresses in loop',
                                          'Step size', 'Mirrored', 'Rate', 'Average runtime'])
    df = df.sort(['Strategy'])

    df.to_csv('strategies.csv')

    conn = sqlite3.connect('strategies.db')
    df.to_sql('strategies', conn)


@click.group()
@click.option('-v', '--verbose', count=True, default=False)
@click.option('-f', '--force', is_flag=True)
@click.option('-c', '--configuration-file', type=click.Path(exists=True), required=True)
@click.option('-x', '--device-configuration-file', type=click.Path(exists=True), required=True)
@click.pass_context
def cli(ctx, verbose, force, configuration_file, device_configuration_file):
    if verbose is True:
        logger.setLevel(logging.DEBUG)

    configuration = parse_configuration(configuration_file)
    if configuration is None:
        ctx.fail('Could not parse configuration file')

    device_configuration = parse_device_configuration(device_configuration_file)
    if device_configuration is None:
        ctx.fail('Could not parse device configuration file')

    ctx.obj['configuration'] = configuration
    ctx.obj['device-configuration'] = device_configuration
    ctx.obj['force'] = force

cli.add_command(cmd_run_strategy)
cli.add_command(cmd_run_strategies)
cli.add_command(cmd_evaluate_strategy)
cli.add_command(cmd_evaluate_strategies)


def main():
    cli(obj={})

if __name__ == '__main__':
    main()
