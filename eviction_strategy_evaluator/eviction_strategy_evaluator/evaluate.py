import os
import pandas as pd
import numpy as np
import logging


def evaluate_strategy_logfile(logfile, device_configuration, threshold):
    # read log file
    try:
        df = pd.read_csv(logfile, dtype={'Miss': float, 'Runtime': float, 'RuntimeBatch': float})
    except:
        return None
    number_of_batches = df.count().RuntimeBatch
    number_of_measurements = df.count().Miss
    # df.loc[len(str(df.Runtime)) > 15] = np.nan
    # df = df.convert_objects(convert_numeric=True)

    # parse filename
    strategy_name = os.path.basename(logfile)[:-4]
    parts = strategy_name.split("-")
    if len(parts) != 5:
        logging.error('Invalid filename: "%s"', logfile)
        return None

    logging.info('Evaluating %s' % strategy_name)
    print('Evaluating %s' % strategy_name)

    number_of_addresses = parts[0]
    number_of_accesses_in_loop = parts[1]
    different_addresses_in_loop = parts[2]
    step_size = parts[3]
    mirrored = True if parts[4] is "M" else False

    # filter outliners
    df = df[np.abs(df-df.mean()) <= (3*df.std())]
    df_count = df.count()

    # calculate eviction rate
    correct_misses = df.Miss[df.Miss > threshold].size
    all_misses = df_count.Miss

    rate = correct_misses / all_misses * 100.0

    # calculate average runtime
    # architecture = device_configuration['device']['arch']
    # if architecture == "armv7": # div64 is enabled by default
    #     df.RuntimeBatch *= 64

    average_runtime = df.Runtime.mean()
    average_runtime_batch = df.RuntimeBatch.mean()
    batch_size = number_of_measurements / number_of_batches

    overhead = average_runtime - (average_runtime_batch / batch_size)

    # print("Average runtime: %d" % average_runtime)
    # print("Average runtime (batch) %d" % average_runtime_batch)
    # print("Average runtime per batch: %d" % (average_runtime_batch / batch_size))
    # print("Number of batches: %d" % number_of_batches)
    # print("Batch size: %d" % batch_size)
    # print("Overhead: %d" % overhead)

    return {
        "rate": rate,
        "average_runtime": average_runtime - overhead,
        "number_of_addresses": number_of_addresses,
        "number_of_accesses_in_loop": number_of_accesses_in_loop,
        "different_addresses_in_loop": different_addresses_in_loop,
        "step_size": step_size,
        "mirroring": mirrored,
        "raw": [strategy_name, number_of_addresses, number_of_accesses_in_loop, different_addresses_in_loop, step_size, mirrored, rate, average_runtime]
        }
