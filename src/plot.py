from matplotlib import pyplot as plt
import seaborn as sns
import numpy as np


def gshare():
    r = open('gshare.results', 'r').readlines()
    r = [x.strip().split() for x in r]
    r = [(int(x[0]), float(x[1])) for x in r]
    r = sorted(r)
    plt.clf()
    sns.set_style('whitegrid')
    plt.title('Gshare')
    plt.figure(figsize=(12, 6))
    plt.xlabel('length of global history')
    plt.ylabel('miss rate (%)')
    xticks = np.arange(5, 31)
    plt.xticks(xticks)
    sns.lineplot(x=xticks, y=[x[1] for x in r])
    plt.savefig('gshare')


def tage():
    r = open('tage.results', 'r').readlines()
    r = [x.strip() for x in r]
    r = [float(x) for x in r]
    plt.clf()
    sns.set_style('whitegrid')
    plt.title('TAGE')
    plt.figure(figsize=(12, 6))
    plt.xlabel('# of components')
    plt.ylabel('miss rate (%)')
    xticks = np.arange(2, 20)
    plt.xticks(xticks)
    sns.lineplot(x=xticks, y=r)
    plt.savefig('tage')


def tournament(t):
    tt = {'g': 'global history', 'l': 'local history', 'i': 'program counter'}
    r = open(f"tournament_{t}.results", 'r').readlines()
    r = [x.strip() for x in r]
    r = [float(x) for x in r]
    plt.clf()
    sns.set_style('whitegrid')
    plt.title('tournament_{t}')
    plt.figure(figsize=(12, 6))
    plt.xlabel('length of {tt[t]}')
    plt.ylabel('miss rate (%)')
    xticks = np.arange(5, 31)
    plt.xticks(xticks)
    sns.lineplot(x=xticks, y=r)
    plt.savefig(f'tournament_{t}')


tournament('g')
tournament('l')
tournament('i')