import matplotlib as mpl
import uproot3 as uproot
import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.ticker import (MultipleLocator, AutoMinorLocator)

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


# Computational Functions
def IPSignificance(row):
    JetPt = row["Jet.PT"]
    JetEta = row["Jet.Eta"]
    JetPhi = row["Jet.Phi"]
    JetConstituents = row["Jet.Particles"]

    TrackPt = row["Track.PT"]
    TrackEta = row["Track.Eta"]
    TrackPhi = row["Track.Phi"]
    TrackD0 = row["Track.D0"]
    TrackErrD0 = row["Track.ErrorD0"]
    TrackDZ = row["Track.DZ"]
    TrackErrDZ = row["Track.ErrorDZ"]
    TrackUID = row["Track.fUniqueID"]

    TrackXd = row["Track.Xd"]
    TrackYd = row["Track.Yd"]
    TrackZd = row["Track.Zd"]
    
    
    #TrackParentFlavor = np.zeros(len(TrackPt))
    JetTrackIPs = []
    for jet in JetPt:
        JetTrackIPs.append([])

    for jet in range(len(JetPt)):
        jet_eta = JetEta[jet]
        if np.abs(jet_eta) > 3.5:
            continue
        jet_phi = JetPhi[jet]
        jet_pt = JetPt[jet]
        
        track_ips = []

        for constituent in JetConstituents[jet]:
            track = -1
            print(constituent)
            print(TrackUID)

            if constituent in TrackUID:
                track = TrackUID.index(constituent) 
            else:
                continue

            track_pt = TrackPt[track]
            if track_pt < 1.0:
                continue

            deltaR = np.sqrt( (TrackEta[track] - JetEta[jet])**2 + (TrackPhi[track] - JetPhi[jet])**2 )
            if deltaR > 0.5:
                continue

            jpx = jet_pt*math.cos(jet_phi)
            jpy = jet_pt*math.sin(jet_phi)
            jpz = jet_pt*math.sinh(jet_eta)

            tx = TrackXd[track]
            ty = TrackYd[track]
            tz = TrackZd[track]
            sign = -1
            if (jpx * tx + jpy * ty + jpz * tz) > 0.0:
                sign = 1

            d0 = TrackD0[track]
            d0_error = TrackErrD0[track]

            dz = TrackDZ[track]
            dz_error = TrackErrDZ[track]

            track_ips.append(sign  * math.fabs( (d0/d0_error)**2 + (dz/dz_error)**2 ))
            
        JetTrackIPs[jet] = track_ips

    return JetTrackIPs

# Computational Functions
def IPSignificanceOld(row):
    JetPt = row["Jet.PT"]
    JetEta = row["Jet.Eta"]
    JetPhi = row["Jet.Phi"]

    TrackPt = row["Track.PT"]
    TrackEta = row["Track.Eta"]
    TrackPhi = row["Track.Phi"]
    TrackD0 = row["Track.D0"]
    TrackErrD0 = row["Track.ErrorD0"]
    TrackDZ = row["Track.DZ"]
    TrackErrDZ = row["Track.ErrorDZ"]
    TrackUID = row["Track.fUniqueID"]

    TrackXd = row["Track.Xd"]
    TrackYd = row["Track.Yd"]
    TrackZd = row["Track.Zd"]
    
    
    #TrackParentFlavor = np.zeros(len(TrackPt))
    TrackIP = np.ones(len(TrackPt))*-999.0

    for jet in range(len(JetPt)):
        jet_eta = JetEta[jet]
        if np.abs(jet_eta) > 3.5:
            continue
        jet_phi = JetPhi[jet]
        jet_pt = JetPt[jet]
        
        for track in np.arange(len(TrackPt)):

            if TrackIP[track] != -999.0:
                continue

            track_pt = TrackPt[track]
            if track_pt < 1.0:
                continue

            deltaR = np.sqrt( (TrackEta[track] - JetEta[jet])**2 + (TrackPhi[track] - JetPhi[jet])**2 )
            if deltaR > 0.5:
                continue

            jpx = jet_pt*math.cos(jet_phi)
            jpy = jet_pt*math.sin(jet_phi)
            jpz = jet_pt*math.sinh(jet_eta)

            tx = TrackXd[track]
            ty = TrackYd[track]
            tz = TrackZd[track]
            sign = -1
            if (jpx * tx + jpy * ty + jpz * tz) > 0.0:
                sign = 1

            d0 = TrackD0[track]
            d0_error = TrackErrD0[track]

            dz = TrackDZ[track]
            dz_error = TrackErrDZ[track]

            TrackIP[track] = sign  * math.fabs( (d0/d0_error)**2 + (dz/dz_error)**2 )

    return TrackIP

def TrackSource(row):
    JetPt = row["Jet.PT"]
    JetEta = row["Jet.Eta"]
    JetPhi = row["Jet.Phi"]
    JetFlavor = row["Jet.Flavor"]

    TrackPt = row["Track.PT"]
    TrackEta = row["Track.Eta"]
    TrackPhi = row["Track.Phi"]

    TrackParentFlavor = np.zeros(len(TrackPt))

    for jet in range(len(JetPt)):
        jet_eta = JetEta[jet]
        if np.abs(jet_eta) > 3.5:
            continue
        jet_pt = JetPt[jet]
        
        for track in np.arange(len(TrackPt)):

            parent_flavor = TrackParentFlavor[track]
            if parent_flavor != -999.0 and (parent_flavor == 4 or parent_flavor == 5):
                continue

            track_pt = TrackPt[track]
            if track_pt < 1.0:
                continue

            deltaR = np.sqrt( (TrackEta[track] - JetEta[jet])**2 + (TrackPhi[track] - JetPhi[jet])**2 )
            if deltaR > 0.5:
                continue

            TrackParentFlavor[track] = JetFlavor[jet]

    return TrackParentFlavor


def histplot(x, xrange, xbins, density=False):
    (counts, bins) = np.histogram(x, range=xrange, 
                                  bins=xbins)

    bin_widths = np.diff(bins)
    bin_centers = bins[:-1] + bin_widths/2

    errors = np.sqrt(counts)
    rel_errors = errors/counts


    # convert counts to dsigma/dpT * 100/fb
    y = counts #/ bin_widths
    if density:
        y = y/len(x)/bin_widths
    y_errors = rel_errors * y
    
    return (bin_centers, bin_widths, y, y_errors)



# Parse arguments
parser = argparse.ArgumentParser()

parser.add_argument("-d", "--dir", type=str,
                    help="Directory containing input files")
parser.add_argument("-i", "--input", type=str,
                    help="Main input subfolder")

args = parser.parse_args()


df = eat.UprootLoad([f"{args.dir}/{args.input}/[0-4]/out.root"], "Delphes", 
                    branches=["Jet.Flavor", "Jet.PT", "Jet.Eta", "Jet.Phi", "Jet.Particles",
                              "Track.fUniqueID", "Track.PT", "Track.Eta", "Track.Phi", "Track.D0", "Track.DZ",
                              "Track.ErrorDZ", "Track.ErrorD0", "Track.Xd", "Track.Yd", "Track.Zd"])
#df = df[:100]

n_gen = len(df)
print(f"n_gen = {n_gen}")

df["Track.IPSignificance"] = df.apply( IPSignificanceOld , axis=1)
df["Track.Source"] = df.apply( TrackSource , axis=1)

print(df.head())


track_ips = np.concatenate(df['Track.IPSignificance'].to_numpy()).ravel()
track_flavor = np.concatenate(df['Track.Source'].to_numpy()).ravel()

matched_ips = track_ips[ track_flavor >= 0 ]
matched_flavor = track_flavor[ track_flavor >= 0 ] 

charm_ips = track_ips[ matched_flavor == 4 ]
light_ips = track_ips[ (matched_flavor < 4) | (matched_flavor == 21)  ]

print(matched_ips)

# Draw the IP significance plot

fig, ax = plt.subplots(figsize=(12,8))
plt.axis('off')


gridspec = fig.add_gridspec(ncols=1, nrows=1, width_ratios=[1], height_ratios=[1])

ax1 = fig.add_subplot(gridspec[0, 0])

ax1.grid(which='both', axis='both')
ax1.xaxis.set_major_locator(MultipleLocator(10))
ax1.xaxis.set_major_formatter('{x:.0f}')

# For the minor ticks, use no labels; default NullFormatter.
ax1.xaxis.set_minor_locator(MultipleLocator(2))


xrange = [-30, 30]
#xbins = np.concatenate( ( np.arange(-30,-5,5),np.arange(-5,5,1),np.arange(5, 30, 5) ) )
#xbins = np.arange(-300,300,1)
xbins = np.concatenate( ( np.arange(-300,-30,10),np.arange(-30,30,1),np.arange(30, 300, 10) ) )

(bins, bin_widths, y, y_error) = histplot(light_ips, xrange=xrange, xbins=xbins, density=True)
ax1.errorbar(bins, y, xerr = bin_widths/2, yerr=y_error, label='light jets', marker='o', ms=10, ls='none', linewidth=2, color='red')

(bins, bin_widths, y, y_error) = histplot(charm_ips, xrange=xrange, xbins=xbins, density=True)
ax1.errorbar(bins, y, xerr = bin_widths/2, yerr=y_error, label='charm jets', marker='D', ms=10, ls='none', linewidth=2, color='blue')


plt.ylabel('$\mathrm{P(sIP_{3D} \, | \, Jet \; Flavor)}$')
plt.xlabel('$\mathrm{sIP_{3D}}$')

plt.title("CC-DIS, 10x275GeV, $Q^2>100\\mathrm{GeV^2}$", fontsize=20)

ax1.set_ylim([1e-6,2e0])
ax1.set_xlim(xrange)

ax1.legend(fontsize=18)

plt.yscale('log')
y_minor = mpl.ticker.LogLocator(base = 10.0, subs = np.arange(2.0, 10.0) * 0.1, numticks = 100)
ax1.yaxis.set_minor_locator(y_minor)
ax1.yaxis.set_minor_formatter(mpl.ticker.NullFormatter())
ax1.yaxis.set_major_locator(mpl.ticker.LogLocator(base = 10.0, subs = np.arange(1.0, 2.0), numticks = 100))


plt.tight_layout()

plt.savefig(f"track_ip_significance_{args.input}.png")
plt.savefig(f"track_ip_significance_{args.input}.pdf")



