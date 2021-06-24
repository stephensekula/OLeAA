#######################################
# Order of execution of various modules
#######################################

set ExecutionPath {
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

module KaonPIDModule Kaon {
}

module ElectronPIDModule Electron {
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
    add branches {Electron} {ChargedElectron} {Kinematics Calorimeter Truth}
    add branches {Jet} {FiducialJet} {Kinematics Truth JetTagging}
}


