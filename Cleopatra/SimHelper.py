#!/usr/bin/env python3

#============== experimental Simulation Helper using PyROOT and PyQT
#
#   need pip install PyQt6
#   need to make at Cleopatra
#
#=====================================================

import sys
from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QApplication, QWidget, QMainWindow, QLabel, QPushButton, QVBoxLayout, QPlainTextEdit, QGroupBox, QGridLayout
import subprocess
import ROOT
import webbrowser

def SaveTxtFromEditor():
  if presentFileName != "" :
    with open(presentFileName, "w") as f:
      f.write(editor.toPlainText())

def LoadTxtToEditor(txtFileName):
  global presentFileName
  SaveTxtFromEditor()

  presentFileName = txtFileName
  with open(txtFileName) as f:
    text = f.read()
    editor.setPlainText(text)

def RunSimulation():
  SaveTxtFromEditor()
  bash_command = "../Cleopatra/SimTransfer reactionConfig.txt detectorGeo.txt 0 '' transfer.root"
  process = subprocess.Popen(bash_command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  output, error = process.communicate()
  if process.returncode != 0:
    print("Error:", error.decode())
  else:
    print("Output:", output.decode())


def OpenHttpServer():
  global ser, file, tree
  file = ROOT.TFile("transfer.root", "read")
  tree = file.Get("tree")

  hEZ = ROOT.TH2F("hEZ", "E - Z; z[mm]; e[MeV]", 400, -600, 0, 400, 0, 10)
  hXY = ROOT.TH2F("hXY", "Array; X[mm]; Y[MeV]", 200, -20, 20, 200, -20, 20)
  hRecoilXY = ROOT.TH2F("hRecoilXY", "Recoil; X[mm]; Y[MeV]", 400, -60, 60, 400, -60, 60)
  hThetaCMZ = ROOT.TH2F("hThetaCMZ", "TheatCM - Z; X[mm]; thetaCM [deg]", 400, -600, 0, 400, 0, 60)
  hExCal = ROOT.TH1F("hExCal", "ExCal; MeV", 400, -1, 5)


  for event in tree:
    if( event.hit != 1 or event.thetaCM < 10 or event.loop != 1) : continue

    hEZ.Fill(event.array_hit_z, event.energy_light) # x, y
    hXY.Fill(event.xArray, event.yArray)
    hRecoilXY.Fill(event.xRecoil, event.yRecoil)
    hThetaCMZ.Fill(event.array_hit_z, event.thetaCM)
    hExCal.Fill(event.ExCal)

  
  ser = ROOT.THttpServer("http:9876")
  # ser.SetJSROOT("https://root.cern.ch/js/latest/")

  ser.Register("/", hEZ)
  ser.Register("/", hXY)
  ser.Register("/", hRecoilXY)
  ser.Register("/", hThetaCMZ)
  ser.Register("/", hExCal)

  ser.SetItemField("/","_layout","grid4x4")
  ser.SetItemField("/","_drawitem","[hEZ, hRecoilXY, hExCal, hThetaCM]")

  webbrowser.open("http://localhost:9876/")


########################################################
if __name__ == "__main__":

  app = QApplication(sys.argv)
  editor = QPlainTextEdit()

  presentFileName = ""

  ser = None
  file = None
  tree = None  

  window = QMainWindow()
  window.setWindowTitle("Simulation Helper")
  window.setFixedSize(800, 1000)

  mainWidget = QWidget()
  window.setCentralWidget(mainWidget)

  layout = QGridLayout()
  mainWidget.setLayout(layout)

  reactionGroup = QGroupBox("Reaction", window)
  layout.addWidget(reactionGroup, 0, 0)
  reactionLayout = QVBoxLayout(reactionGroup)

  bDetGeo = QPushButton("Detector Geo", reactionGroup)
  reactionLayout.addWidget(bDetGeo)
  bDetGeo.clicked.connect(lambda : LoadTxtToEditor("detectorGeo.txt"))

  bReactionConfig = QPushButton("Reaction Config", reactionGroup)
  reactionLayout.addWidget(bReactionConfig)
  bReactionConfig.clicked.connect(lambda : LoadTxtToEditor("reactionConfig.txt"))

  bSim = QPushButton("Simulation", reactionGroup)
  reactionLayout.addWidget(bSim)
  bSim.clicked.connect(RunSimulation)

  bTest = QPushButton("Open Browser", reactionGroup)
  reactionLayout.addWidget(bTest)
  bTest.clicked.connect(OpenHttpServer)

  layout.addWidget(editor, 0, 1, 5, 5)
  LoadTxtToEditor("detectorGeo.txt")

  # # Create PyQtGraph plot
  # plot = PlotWidget()
  # plot_item = plot.getPlotItem()
  # plot_item.plot(x=[1, 2, 3], y=[4, 6, 2])
  # layout.addWidget(plot)

  # Show the window and start the event loop
  window.show()
  sys.exit(app.exec())