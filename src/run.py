import os
import subprocess as sp
import numpy as np

base = {
    'fp_1': [0.825, 0.991],
    'fp_2': [1.678, 3.246],
    'int_1': [13.839, 12.622],
    'int_2': [0.420, 0.426],
    'mm_1': [6.696, 2.581],
    'mm_2': [10.138, 8.843]
}


def run(trace, args):
    c = sp.Popen(f'bunzip2 -kc {trace} | ./predictor {args}',
                 stdout=sp.PIPE,
                 shell=True)
    out = c.communicate()[0].decode()
    out = [[y.strip() for y in x.strip().split(':')] for x in out.split('\n')
           if x.strip()]
    out = dict(out)
    return out


traces = os.listdir('../traces')
traces = [os.path.join('../traces', x) for x in traces]
rates = []
for trace in traces:
    t = trace.split('/')[-1].split('.')[0]
    print(t, trace)
    # out = run(trace, '--gshare:13')
    # out = run(trace, f'--tournament:{i}:10:10')
    # out = run(trace, f'--tournament:9:{i}:10')
    # out = run(trace, f'--tournament:9:10:{i}')
    # out = run(trace, '--tournament:9:10:10')
    out = run(trace, '--custom')
    print(out)
    print(np.array(base[t]) > float(out['Misprediction Rate']))
    rates.append(float(out['Misprediction Rate']))
print(np.mean(rates))
