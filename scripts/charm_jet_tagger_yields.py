import matplotlib as mpl
import uproot
import matplotlib.pyplot as plt
import scipy
import numpy as np
import math
import pandas as pd
import seaborn as sns
import mplhep as hep
#import zfit
import inspect
import sys
import argparse 
import glob

from concurrent.futures import ThreadPoolExecutor

plt.style.use(hep.style.ATLAS)

plt.rcParams.update({'font.sans-serif': "Arial",
                     'font.family': "sans-serif",
                     'font.size': 30,
                     'mathtext.fontset': 'custom',
                     'mathtext.rm': 'Arial',
                     })


import EICAnalysisTools as eat


branchlist=["*"]

print("Loading data...")

df = eat.UprootLoad([f"../CC_DIS_e10_p275_CT18NNLO/*/out.root"], "tree", branches=branchlist)

# define the cuts for the study

jet_sip3dtag = np.array(df["jet_sip3dtag"].flatten())
jet_ktag = np.array(df["jet_ktag"].flatten())
jet_etag = np.array(df["jet_etag"].flatten())
jet_mutag = np.array(df["jet_mutag"].flatten())
jet_flavor = np.array(df["jet_flavor"].flatten())

cut_sip3d_tagged = jet_sip3dtag == 1
cut_ktagged   = jet_ktag == 1
cut_etagged   = jet_etag == 1
cut_mutagged  = jet_mutag == 1

cut_lightjets = (jet_flavor < 4) | (jet_flavor == 21)
cut_charmjets = jet_flavor == 4


cut_map = {}
cut_map["sIP3D-tagged"] = cut_sip3d_tagged
cut_map["sIP3D-untagged"] = np.invert(cut_sip3d_tagged)
cut_map["sIP3D-untagged, e-tagged"] = np.invert(cut_sip3d_tagged) & (cut_etagged)
cut_map["sIP3D-untagged, mu-tagged"] = np.invert(cut_sip3d_tagged) & (cut_mutagged) 
cut_map["sIP3D-untagged, K-tagged"] = np.invert(cut_sip3d_tagged) & (cut_ktagged)
cut_map["sIP3D-untagged, (e|mu|K)-tagged"] = np.invert(cut_sip3d_tagged) & (cut_mutagged | cut_etagged | cut_ktagged)
cut_map["Tagged by anything"] = (cut_sip3d_tagged | cut_mutagged | cut_etagged | cut_ktagged) 

for jet_type in (cut_lightjets, cut_charmjets):
    n_all = len(jet_flavor[ jet_type ])
    print(n_all)
    for name in cut_map:
        tag_cat = cut_map[name]
        n_select = len(jet_flavor[ jet_type & tag_cat ])
        print(f"{name}: {n_select} ({n_select/n_all*100:.3f}%)")
    print("========================================")    


