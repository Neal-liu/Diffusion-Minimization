# Diffusion-Minimization
Diffusion Minimization on Specific Targets in Social Networks

## Introduction
* This is my research, and there are three algorithms implements :
  1. Baseline : implements with greedy.
  2. LDT : improve Baseline, including two pruning mechanisms : 
    - threshold
    - sorted list
  3. Community-based : First research about diffusion minimization by Zongqing et al, INFOCOM 2014.
  
* Query : k, labels

## Building and Running
  * build Baseline algorithm
```
  make diffusion1
```
  * build LDT algorithm
```
  make diffusion2
```
  * build Community-based algorithm
```
  make diffusion3
```
- If the dataset is synthetic, it means we have to read the "communities.txt" file for giving each node's community. So we have to add parameter after command, for example : 
```
  make diffusion1 COM=yes
  /* ps. $(COM) default is no. */
```

After build ELF executable, we have to choose the dataset to run :
* run twitter data, facebook data, epinions data, wiki-Vote data :
```
  make twitter
  make facebook
  make epinions
  make wiki
```
* run synthetic data (including 1,000 nodes, 5,000 nodes, 10,000 nodes, 40,000 nodes) :
```
  make 1000
  make 5000
  make 10000
  make 40000
```
* If you want to change the `k` value, you have to edit Makefile by your own.
