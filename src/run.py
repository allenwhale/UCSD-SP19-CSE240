import os
import subprocess as sp
import numpy as np


def run(trace, args):
    c = sp.Popen(
        f'bunzip2 -kc {trace} | ./predictor {args}',
        stdout=sp.PIPE, shell=True)
    out = c.communicate()[0].decode()
    out = [[y.strip() for y in x.strip().split(':')]
           for x in out.split('\n') if x.strip()]
    out = dict(out)
    return out


traces = os.listdir('../traces')
traces = [os.path.join('../traces', x) for x in traces]
rates = []
for trace in traces:
    print(trace)
    out = run(trace, '--gshare:13')
    # out = run(trace, '--tournament:9:10:10')
    print(out)
    rates.append(float(out['Misprediction Rate']))
print(np.mean(rates))
