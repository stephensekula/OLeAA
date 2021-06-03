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
import pickle
import os

from concurrent.futures import ThreadPoolExecutor

plt.style.use(hep.style.ATLAS)

plt.rcParams.update({'font.sans-serif': "Arial",
                     'font.family': "sans-serif",
                     'font.size': 30,
                     'mathtext.fontset': 'custom',
                     'mathtext.rm': 'Arial',
                     })

import EICAnalysisTools as eat


# Parse arguments
parser = argparse.ArgumentParser()

parser.add_argument("-d", "--dir", type=str,
                    help="Directory containing input files")
parser.add_argument("-n", "--input", type=str,
                    help="Name of input sample folder")
parser.add_argument("-x", "--xvar", type=str, default='jet_p',
                    help="jet_pt, jet_p, etc.")
parser.add_argument("-r", "--redraw", action='store_true',
                    help="Force redraw of figure from raw data instead of cached (pickled) figure")


parser.set_defaults(redraw=False)

args = parser.parse_args()

pt_name = "Jet.PT"
eta_name = "Jet.Eta"
flavor_name = "Jet.Flavor"

if args.xvar == "genjet_p":
    pt_name = "GenJet.PT"
    eta_name = "GenJet.Eta"
    flavor_name = "GenJet.Flavor"
    
branchlist=[pt_name, eta_name, flavor_name]

print("Loading data...")

figure_file_suffixes = ["png","pdf"]

figure_file_prefix = f"charm_jet_coverage_{args.xvar}_{args.input}"

redraw_from_raw = False


if args.redraw == True or (not os.path.isfile(figure_file_prefix+".pkl")):
    redraw_from_raw = True

print(f"Redraw figure from raw data? {redraw_from_raw}")


the_plot = None
the_axes = None


xlabel = "Jet $p_T$ [GeV]"
if args.xvar == "jet_pt":
    xlabel = "Charm Jet $p_T$ [GeV]"
elif args.xvar == "jet_p":
    xlabel = "Charm Jet Momentum [GeV]"
elif args.xvar == "genjet_p":
    xlabel = "Generator-Level Charm Jet Momentum [GeV]"
else:
    print("Unknown x variable")
    sys.exit()

if redraw_from_raw:
    df = eat.UprootLoad([f"{args.dir}/{args.input}/*/out.root"], "Delphes", branches=branchlist)

    #df = df[:10000]

    n_gen = len(df)
    print(f"n_gen = {n_gen}")


    jet_pt = np.concatenate(df[pt_name].to_numpy()).ravel()
    jet_eta = np.concatenate(df[eta_name].to_numpy()).ravel()
    #jet_tag = np.concatenate(df['GenJet.BTag'].to_numpy()).ravel()
    jet_flavor = np.concatenate(df[flavor_name].to_numpy()).ravel()
    jet_theta = 2*np.arctan(np.exp(-jet_eta))
    jet_p = jet_pt*np.cosh(jet_eta)
    
    #jet_tagged = (jet_tag == 1)
    charm_flavor = ( jet_flavor == 4 )



    angles = np.radians(np.linspace(0, 180, 90))

    mom = np.linspace(0,100,10)
    xvals = jet_pt[charm_flavor]
    thetavals = jet_theta[charm_flavor]
    
    if args.xvar == "jet_pt":
        mom=np.linspace(0,50,10)
        xlabel = "Charm Jet $p_T$ [GeV]"
        xvals = jet_pt[charm_flavor]
    elif args.xvar == "jet_p":
        mom=np.linspace(0,80,16)
        xlabel = "Charm Jet Momentum [GeV]"
        xvals = jet_p[charm_flavor]
    elif args.xvar == "genjet_p":
        mom=np.linspace(0,80,16)
        xlabel = "Generator-Level Charm Jet Momentum [GeV]"
        xvals = jet_p[charm_flavor]
    else:
        print("Unknown x variable")
        sys.exit()


    # Make the plot
    the_plot, the_axes = plt.subplots(1,1,figsize=(8,8),subplot_kw=dict(projection='polar'),dpi=300)


    values, thetaedges, redges = np.histogram2d(thetavals, xvals, bins=[angles, mom])
    r, theta = np.meshgrid( redges[:-1], thetaedges[:-1])

    #ax.contourf(theta, r, values,levels=18)
    the_axes.contourf(theta, r, values)
    list=[0,np.pi/6,np.pi/3,np.pi/2,4*np.pi/6,5*np.pi/6,np.pi]
    the_axes.set_xticks(list)
    the_axes.set_thetamin(0)
    the_axes.set_thetamax(180)
    plt.xlabel(xlabel,labelpad=-75,fontsize=22)
    plt.title(f"CC-DIS, 10GeVx275GeV, $Q^2>100\\mathrm{{GeV^2}}$", fontsize=22)
    plt.text(3.65/2,np.max(mom)+12.5,'Polar Angle',fontsize=22,multialignment='right')
    the_axes.tick_params(axis='x', labelsize=18 , pad=15)
    the_axes.tick_params(axis='y', labelsize=18 , pad=10)
    plt.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=0, hspace=0)
    plt.tight_layout()

    # archive the plot
    with open(f"{figure_file_prefix}.pkl", 'wb') as f: 
        pickle.dump(the_plot, f) 

else:
    dummy = plt.figure()
    with open(f"{figure_file_prefix}.pkl", 'rb') as f:
        the_plot = pickle.load(f)
    the_axes = plt.gca()
    the_plot.show()


    # mod the plot!
    #plt.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=0, hspace=0)
    #the_axes.margins(2,2)
    #plt.xlabel(xlabel,labelpad=-75,fontsize=22)
    #the_axes.margins(x=0,y=0)
    #the_axes.autoscale_view('tight')
    the_plot.show()


for suffix in figure_file_suffixes:
    plt.savefig(f"{figure_file_prefix}.{suffix}", bbox_inches = 'tight', pad_inches = 0.1)

