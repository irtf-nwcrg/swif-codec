#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Sep 19 10:03:41 2019

@author: Oumaima Attia
"""

#%%

import numpy as np


#%%

np.random.seed(2)

w = 6
q = 0.1
λ = 1 - pow(1-q,w)
nb_simul = 1
nb_win = 100000

losses = np.random.random(nb_win*(w)) < q
losses_per_window = np.array([sum(window_losses) for window_losses in np.array_split(losses, nb_win)]) 

c_0 = len(np.where(losses_per_window > 0)[0])
c1 = len(np.where(losses_per_window == 1)[0]) 
c2 = len(np.where(losses_per_window == 2)[0]) 

c_0 / len(losses_per_window), c1/c_0, c2/c_0

#%%
p_0 = 1-pow(1-q,w) # w : w-1 packets + one repair packet
#	/* The probability of exactly 1 loss given the condition of having a non zero loss */
p1 = w*q*pow(1-q, w-1)/p_0
#	/* The probability of exactly 2 losses given the condition of having a non zero loss */
p2 = w*(w-1)/2*pow(q,2)*pow(1-q, w-2)/p_0

p_0, p1, p2

#%%

zero_pos_array = np.where(losses_per_window == 0)[0]
print(losses_per_window)
sequence_list = np.split(losses_per_window, zero_pos_array)
sequence_list[0] = np.concatenate((np.array([0]), sequence_list[0]))

info_per_k = {}
D1 = 0
D2_1 = 0
D2_2 = 0 
D3 = 0
print("sequence_list", sequence_list)

for sequence in sequence_list:
    k = len(sequence)-1
    #print(sequence, k, np.where(sequence==2)[0])
    if k not in info_per_k:
        info_per_k[k] = {"count": 0 }
    has_more_than_two = len(np.where(sequence > 2)[0])>0
    count_two = len(np.where(sequence == 2)[0])
    if has_more_than_two:
        D3 += 1 # k
        # we cannot decode
        pass
    elif count_two == 1:
        # can decode
        D2_1 += 1 # k
        pass
    elif count_two > 1:
        # cannot decode
        D2_2 += 1 # (np.where(sequence==2)[0][0])
        pass
    else:
        # can decode
        D1 += 1 # k
        pass
    info_per_k[k]["count"] += 1

info_per_k
print("D1",D1)
print("D2_1",D2_1)
print("D2_2",D2_2)
print("D3",D3-D2_2)


#%%

nb_sequence = sum(info["count"] for info in info_per_k.values())
assert nb_sequence == len(sequence_list)
print(D1/nb_sequence)
print(D2_1/nb_sequence)
print(D2_2/nb_sequence)
print(D3/nb_sequence)
#print((D1+D2_1+D2_2)/ nb_sequence)

Sk = 0
d11 = 0
d21 = 0 
d22 = 0 
for k in sorted(info_per_k.keys()):
    Pr = pow(λ,k)*(1-λ)
    print(k, info_per_k[k], end=" ")
    print(info_per_k[k]["count"] / nb_sequence)
    d1 = pow(p1,k) * Pr
    d11 += d1
    d2_1 = k*pow(p1,k-1) * p2 * Pr
    d21 += d2_1
    d2_2 = (p2 * ((1-pow(p1,k))/(1-p1)) - p2*pow(p1,k-1)*k) * Pr
    d22 += d2_2
    #Sk += Pr * (d1+d2_1+d2_2)
print("Sk",Sk)
print(d11)
print(d21)
print(d22)
print(1-d11-d21-d22)


#%%

nb_sequence


#%%

sequence = np.array([0,1,2,3,2,3,0])
(np.where(sequence==2)[0][0])
