import pandas as pd
import numpy as np
import json
import sys
import random
from copy import deepcopy

'''
TrafficFactor = 0.1
SplitFactor = 0.3 
Mode = 0
'''

if (len(sys.argv) < 6):
    print("Insufficient parameters: Please enter all required parameters")
    print('python', sys.argv[0], "csvFile outFile StartHour TrafficFactor SplitFactor mode")
    exit()

lim = 4
sum_of_arr1 = []
sum_of_arr2 = []
sum_of_arr3 = []
col_no = [1, 5, 9, 13, 17, 21]

start = int(sys.argv[3]) * 60*60
end = start + 60*60
mode = int(sys.argv[6]) if len(sys.argv)>6 else 0
factor = float(sys.argv[4])
split_fact = float(sys.argv[5])
outFile = sys.argv[2]

route = [["road_2_1_2", "road_1_1_3"], ["road_0_1_0", "road_1_1_0"], ["road_1_0_1", "road_1_1_2"], ["road_2_1_2", "road_1_1_2"]]

t1 = open(outFile, 'w')
print('[', file=t1)

df = pd.read_csv(sys.argv[1], header=None)

# Converting dataframe structure to numpy array
arr1 = df.values

if mode == 0:
    sum_of_arr1 = arr1[:, col_no[0]] + arr1[:, col_no[1]]
    sum_of_arr2 = arr1[:, col_no[2]] + arr1[:, col_no[3]]
    sum_of_arr3 = arr1[:, col_no[4]] + arr1[:, col_no[5]]
elif mode == 1:
    sum_of_arr1 = arr1[:, col_no[0]]
    sum_of_arr2 = arr1[:, col_no[2]]
    sum_of_arr3 = arr1[:, col_no[4]]
else:
    sum_of_arr1 = arr1[:, col_no[1]]
    sum_of_arr2 = arr1[:, col_no[3]]
    sum_of_arr3 = arr1[:, col_no[5]]
arr = [sum_of_arr1, sum_of_arr2, sum_of_arr3]


def fn(arr, i, lim):
    sum = 0
    for l in range(lim):
        diff = arr[i+l+1] - arr[i+l]
        if (diff > 0):
            sum += diff
    return sum

dumping_var = {"vehicle": {"length": 2.0, "width": 2.0, "maxPosAcc": 2.0,
                           "maxNegAcc": 4.5, "usualPosAcc": 2.0, "usualNegAcc": 4.5,
                           "minGap": 2.5, "maxSpeed": 11.111, "headwayTime": 2},
               "route": route[0], "interval": 1.0, 
               "startTime": 0, "endTime": 0}

total_num_veh = 0
for i in range(start, end-lim, 5):
    dumping_var["startTime"] = i - start
    dumping_var["endTime"] = i - start

    for l in range(len(arr)):
        no_of_vehicles = int(fn(arr[l], i, lim) / factor)
        if (no_of_vehicles > 0):
            total_num_veh += no_of_vehicles
            for k in range(no_of_vehicles):
                if not l and (random.random() > split_fact):
                    dumping_var["route"] = route[3]
                else:
                    dumping_var["route"] = route[l]
                json.dump(dumping_var, t1)
                print(',', file=t1)

print(']', file=t1)

t1.close()

print(total_num_veh, 'vehicles written to', outFile)
