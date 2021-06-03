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
parser.add_argument("-i", "--input", type=str,
                    help="Main input subfolder")
parser.add_argument("-x", "--xvar", type=str, default='pt',
                    help="pt [default], eta, bjorken_x")

args = parser.parse_args()



# Units
global u_fb, u_mb, xsection
u_fb = 1
u_mb = 1e12*u_fb
u_pb = 1e3*u_fb
#xsection = 2.408e-08*u_mb


draw_config={}
if args.xvar == 'eta':
    draw_config['xvar'] = 'jet_eta'
    draw_config['xrange'] = [-4.0, 4.0]
    draw_config['xbins'] = [-4.0, -3.5, -2.5, -1.0, 1.0, 2.5, 3.5, 4.0]
    draw_config['ylimits'] = [1, 1e3]
    draw_config['xlimits'] = [-5,5]
    draw_config['yunits'] = ''
    draw_config['xunits'] = ''
elif args.xvar == 'pt':
    draw_config['xvar'] = 'jet_pt'
    draw_config['xrange'] = [10,50]
    draw_config['xbins'] = [10,12.5,15,20,25,35,60]
    draw_config['ylimits'] = [1, 1e3]
    draw_config['xlimits'] = [0,60]
    draw_config['yunits'] = '[$\\mathrm{GeV^{-1}}$]'
    draw_config['xunits'] = '[GeV]'
elif args.xvar == 'bjorken_x':
    draw_config['xvar'] = 'bjorken_x'
    draw_config['xrange'] = [1e-3,1]
    #draw_config['xbins'] = np.concatenate([np.arange(5e-3,1e-2,1e-3),np.arange(1e-2,1e-1,1e-2),np.arange(1e-1,1.1,1e-1)])
    draw_config['xbins'] = [5e-3, 1e-2, 1e-1, 1]
    draw_config['ylimits'] = [1, 1e3]
    draw_config['xlimits'] = [1e-3,1]
    draw_config['yunits'] = ''
    draw_config['xunits'] = ''


#DrawDiffTagYieldPlot(df, draw_config)


xvar=draw_config['xvar']
xrange=draw_config['xrange']
xbins=draw_config['xbins']
ylimits=draw_config['ylimits']
xlimits=draw_config['xlimits']
yunits=draw_config['yunits']
xunits=draw_config['xunits']

branchlist=["*"]

print("Loading data...")

df = eat.UprootLoad([f"{args.dir}/{args.input}/*/out.root"], "tree", branches=branchlist)
#df = df.apply(pd.Series.explode)

#df = df[:50000]

ucr_blue = "#2D6CC0"
ucr_gold = "#F1AB00"
ucr_gray = "#393841"

# Flavour tagging study code
def DifferentialTaggingYield(df, x='Jet.PT', xrange=[10,50], xbins=[10,12.5,15,20,25,30,40,50], which='all', process='CC_DIS_e10_p275_CT18NNLO'):
    global u_fb, u_mb,  n_gen
    n_gen = len(df)
    print(f"n_gen = {n_gen}")

    jet_flavor = df['jet_flavor']
    jet_tag = df['jet_sip3dtag']

    jet_x = df[x]

    if which == 'charm':
        jet_x = jet_x[ (jet_tag == 1) & (jet_flavor == 4) ]
    elif which == 'light':
        jet_x = jet_x[ (jet_tag == 1) & ((jet_flavor < 4) | (jet_flavor == 21)) ]

    (counts, bins) = np.histogram(jet_x.flatten(), range=xrange, 
                                  bins=xbins)
                                  #bins=40)


    print("Raw Counts:")
    print(counts)
    print("Raw Efficiency:")
    print(counts/n_gen)

    bin_widths = np.diff(bins)
    bin_centers = bins[:-1] + bin_widths/2

    errors = np.sqrt(counts)
    rel_errors = errors/counts


    # convert counts to dsigma/dpT * 100/fb
    print(f"Normalizing yields to x-section {eat.TotalXSection(process):.3f}fb")
    dsigma_dx = counts * eat.TotalXSection(process)  * 100*u_fb**(-1) / n_gen #/ bin_widths
    dsigma_dx_errors = rel_errors * dsigma_dx
    
    return (bin_centers, bin_widths, dsigma_dx, dsigma_dx_errors)





process='CC_DIS_e10_p275_CT18NNLO'
charm_ct18nnlo = DifferentialTaggingYield(df, x=xvar, xrange=xrange, xbins=xbins, which='charm', process=process)

print(charm_ct18nnlo)


polarization_e=0.70
polarization_p=0.70
polarization=0.70 # single beam polarization

N = charm_ct18nnlo[2]
errN = np.zeros(len(N))

for index in range(len(errN)):
    errN[index] = np.sqrt(N[index]*(1+polarization_e)/2.0)

print("Yields")
print(N)
print("Errors")
print(errN)
print("Total Events:")
print(np.sum(N))


dA = 1.0/(errN*polarization_p)*100


fig, ax = plt.subplots(figsize=(12,8))
plt.axis('off')


gridspec = fig.add_gridspec(ncols=1, nrows=1, width_ratios=[1], height_ratios=[1])

bins = charm_ct18nnlo[0]
bin_widths = charm_ct18nnlo[1]

zero_line = np.zeros(len(bins))

# Log plot of differential cross-sections
ax1 = fig.add_subplot(gridspec[0, 0])

ax1.grid(which='both', axis='both')

bandlabel=f"$\delta A$ ($p={polarization*100:.0f}\%$)"

ax1.axhline(1.0, color='k', linestyle='-', linewidth=2)
errorboxes = [mpl.patches.Rectangle((x - xe, y - ye), 2*xe, 2*ye, facecolor=ucr_blue, alpha=0.8, 
                                    label=bandlabel)
              for x, y, xe, ye in zip(bins, zero_line, (bin_widths/2), dA)]

# Create patch collection with specified colour/alpha
pc = mpl.collections.PatchCollection(errorboxes, facecolor=ucr_blue, alpha=0.8, label=bandlabel)

# Add collection to axes
ax1.add_collection(pc)


ax1.set_ylim([-100, 100])
ax1.set_xlim(xlimits)
plt.ylabel('Asymmetry Uncertainty [%]')

xvar_symbol="p_T"
if xvar.find('Eta') != -1:
    xvar_symbol='\\eta'
    
plt.xlabel(f'Reconstructed Jet ${xvar_symbol}$ {xunits}')

ax1.legend(handles=[errorboxes[0]])


plt.tight_layout()

plt.savefig(f"dA_100fb_{xvar}_{args.input}.png")
plt.savefig(f"dA_100fb_{xvar}_{args.input}.pdf")
