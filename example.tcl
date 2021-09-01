#######################################
# Order of execution of various modules
#######################################

set ExecutionPath {
    CaloCorrection
    TaggingElectron
    Kaon
    Electron
    FiducialJet
    TreeWriter
}

module ElectronRefinerModule TaggingElectron {
    set inputList Electron
    set outputList TaggingElectron
    add selectors "PT 5.0:1000.0"
    add selectors "Eta -4.0:4.0"
}

module CaloEnergyCorrectorModule CaloCorrection {
    set inputTrackList EFlowTrack
    set inputTowerList Tower
    set outputEMFractionMap EMFracMap
}

module KaonPIDModule Kaon {
}

module ElectronPIDModule Electron {
    set inputList EFlowTrack
    set outputList ChargedElectron
    set fEM_min 0.991
}

module JetRefinerModule FiducialJet {
    set inputList Jet
    set outputList FiducialJet
    add selectors "PT 5.0:1000.0"
    add selectors "Eta -3.0:3.0"
}

module TreeWriterModule TreeWriter {
    add branches {Event} {} {MET DIS}
    add branches {Electron} {TaggingElectron} {Kinematics Truth}
    add branches {Track} {Track} {Kinematics Truth}
    add branches {Track} {ChargedKaon} {Kinematics PID Truth}
    add branches {Track} {barrelDIRCTrack} {Kinematics PID Truth}
    add branches {Track} {mRICHTrack} {Kinematics PID Truth}
    add branches {Track} {dualRICHagTrack} {Kinematics PID Truth}
    add branches {Track} {dualRICHcfTrack} {Kinematics PID Truth}
    add branches {Electron} {ChargedElectron} {Kinematics Calorimeter Truth}
    add branches {Jet} {FiducialJet} {Kinematics Truth JetTagging}
}


