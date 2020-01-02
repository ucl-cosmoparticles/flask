#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
USAGE:   check_recovCls.py  </path/to/flask.config>
EXAMPLE: check_recovCls.py lexample.config

This script takes a flask configuration file and compares the input 
theory Cls to the output recov-cls. The output for this script are plots
in .pdf format outputed in the same folder as the .config 
(or in the current directory if no permissions to write in such folder).

If DENS2KAPPA was used, you need to run Dens2KappaCls first and point the code
to the correct directory for these files.

Plots show the recov cls, the input cls, and the relative error between them.


FIXME: Issues with creating the output directory
FIXME: Automate Dens2KappaCls calculation if not found.

Written by Arthur Loureiro (arthur.loureiro.14@ucl.ac.uk) - Sept/2019
"""
from __future__ import division, print_function
import sys
import numpy as np
import pylab as plt
import matplotlib as mpl
import os
import glob
from scipy.interpolate import InterpolatedUnivariateSpline as intp
from matplotlib import gridspec
import shutil as shu


def create_output_folder(config):
    """
    Creates an output folder for the plots.
    If folder already exists, creates a new one.
    
    Returns the name of the output folder!
    """
    pathtoConfig = os.path.dirname(config)
    rootName = os.path.splitext(os.path.basename(config))[0]
    
    newFolderName = "/Check_RecovCls-" + rootName
    newFolder = os.path.join(pathtoConfig, newFolderName)
    
    if os.path.isdir(newFolder) == True:
        print("Folder ", newFolder, " exists.")
    else:
        try:
            print("Creating folder ", newFolder, " for the outputs.")
            os.mkdir(newFolder)
            
        except:
            print("Cannot create folder at ", pathtoConfig, "! \nWill create folder in current directory!")
            newFolder = "."+newFolderName
            if os.path.isdir(newFolder) == True:
                print("Folder ", newFolder, " exists.")
            else:
                os.mkdir(newFolder)
    
    return newFolder  
    

def search_flask_args(pathFlaskFile, argName):
    """
    returns the argument used in the config file for flask
    """
    with open(pathFlaskFile) as search:
        for line in search:
            #line = line.rstrip().replace(" ", "")
            line = line.split("#")[0]
            if argName in line: #FIXME: need to find exact match!
                print(line)
                args = line.split(':')[1].rstrip().replace(" ", "").replace("\t", "").split('#')[0]
                break # since I can't find a way to find exact match
    
    if args[0]=='0':
        print("Argument not given to flask!")
        return 0
    else:
        return args

def get_config_path(config):
    """
    Gets the flask config path
    """
    dirname = os.path.dirname(config)
    return dirname

def get_recov_cls_dict(config):
    """
    Returns the RecovCls in an array
    """
    
    configDirName = get_config_path(config)
    
    recovClsPathFromConfig = search_flask_args(config, "RECOVCLS_OUT")
    
    if recovClsPathFromConfig is 0:
        print("No recovCls were calculated by Flask. \nDiagnosis cannot be performed!\nExiting the script...")
        sys.exit(-1)
    else:
        #check if the path is absolute:
        if recovClsPathFromConfig[0] == '/':
            RecovClsPath = recovClsPathFromConfig
        else:
            RecovClsPath = os.path.join(configDirName, recovClsPathFromConfig)
        
        assert os.path.isfile(RecovClsPath), "Cannot find the config file!"
        
        RecovCls = np.loadtxt(RecovClsPath)
        
        # getting the header information to create the dictionary:
        with open(RecovClsPath) as header:
            for line in header:
                names = line.replace("Cl-", "").rstrip().split(' ')
                names.remove('#')
                break
        #RecovCls Dictionary
        DictRecovCls = {}

        for n, i in zip(names, RecovCls.T):
            DictRecovCls[n] = i
        
    return DictRecovCls

def checkDens2Kappa(config):
    """
    checks if density2kappa was used.
    """
    
    return bool(search_flask_args(config, "DENS2KAPPA"))

def copy_inputs(config, kappaPath):
    """
    if dens2kappa used, copies the original density cls to the kappa LoS folder
    with the same root name if needed.
    """
    
    inputClsPath = search_flask_args(config, "CL_PREFIX")
    configDirName = get_config_path(config)
    
    print(" >> Copying input density Cls to Kappa Folder!")
    if inputClsPath is 0:
        print("No input cls in the Flask config! \nDiagnosis cannot be performed!\nExiting the script...")
        sys.exit(-1)
    else:
        #check if the path is absolute:
        if inputClsPath[0] == '/':
            inputClsPath = inputClsPath
        else:
            inputClsPath = os.path.join(configDirName, inputClsPath)
        inputFiles = glob.glob(inputClsPath + "*.dat")
        print(inputClsPath)
        for f in inputFiles:
            sufix = f.split(inputClsPath)[1]
            print("Copying ", f, " to ", kappaPath + sufix)
            shu.copy2(f, kappaPath + sufix)
    
def get_input_cls_dict(config, RecovDict, kappaPath=None):
    """
    recovers just the list of files!
    """
    configDirName = get_config_path(config)
    
    if kappaPath is None:
        ClsPath = search_flask_args(config, "CL_PREFIX")
    else:
        print("TRANSFERING DENSITY CLS TO KAPPA FOLDER!")
        ClsPath = kappaPath
        copy_inputs(config, kappaPath)
        #sys.exit(-1)
    
    if ClsPath is 0:
        print("No input cls in the Flask config! \nDiagnosis cannot be performed!\nExiting the script...")
        sys.exit(-1)
    else:
        #check if the path is absolute:
        if ClsPath[0] == '/':
            ClsPath = ClsPath
        else:
            ClsPath = os.path.join(configDirName, ClsPath)
            
        InputClsDict = {}
        for k in RecovDict.keys():
            FileCl = glob.glob(ClsPath + k + ".dat")
            
            if FileCl != []:
                print("Reading input cls from: ", FileCl)
                assert os.path.isfile(FileCl[0]), "Cannot find the file for input cls"
                ell, cls = np.loadtxt(FileCl[0], unpack=1)
                InputClsDict['l'] = ell
                InputClsDict[k] = cls
        
        return InputClsDict

def check_ell_range(recovcls, inputcls):
    """
    check if ell ranges are the same for input and recov:
    True = Same Ell range
    False = Different Ell range
    """
    
    if (recovcls['l'].min() == inputcls['l'].min()) and (recovcls['l'].max() == inputcls['l'].max()):
        return True
    else:
        print("Different ell ranges.") 
        print("\nRecov: ", recovcls['l'].min(), " ", recovcls['l'].max())
        print("\nInput: ", inputcls['l'].min(), " ", inputcls['l'].max())
        return False

def get_nside(config):
    """
    returns the nside
    """
    return int(search_flask_args(config, "NSIDE"))
        
def bin_cls(ell, cls, delta_ell):
    """
    bin the cls in C_b = \sum (2*l +1) C_l / \sum (2*l +1)
    """
    
    ell_bin = np.zeros(int(len(ell)/delta_ell))
    cls_bin = np.zeros_like(ell_bin)
    var_bin = np.zeros_like(ell_bin)
    
    for j in range(len(ell_bin)):
        lmin = j*delta_ell
        lmax = (j + 1)*delta_ell
        ell_range = ell[lmin : lmax]
        
        ellFac = (2*ell_range + 1)
        
        ell_bin[j] = np.sum(ell_range)/delta_ell
        cls_bin[j] = np.sum(ellFac*cls[lmin:lmax])/np.sum(ellFac)
        var_bin[j] = np.std(cls[lmin:lmax])
        
    return ell_bin, cls_bin, var_bin    
    
def Dens2KappaUsed(config, recovClsDict):
    """
    In case Dens2Kappa was used, asks the user if Cls where Calculated,
    if not, it calculates it for the user.
    """
    print("Dens2Kappa used in the .config! Need to calculate Kappa-Cls to compare with.")
    UserAnswer1 = input("Have you previously calculated the K-Cls using Dens2KappaCls? [Y/N] ").upper()
    
    if UserAnswer1 == "Y":
        pathToKappaCls = input("Please provide the path to the Kappa Cls and their root name: ")
        
        # check if there are files in the kappa folder:
        kappaClsFiles = glob.glob(pathToKappaCls + "*.dat")
        
        assert len(kappaClsFiles) != 0, "No Kappa-Cls found in the directory!"
        
        return get_input_cls_dict(config, recovClsDict, kappaPath=pathToKappaCls)
        
    elif UserAnswer1 == "N":
        print("Please calculate them with Dens2KappaCls!") #FIXME calculate automatically instead
        sys.exit(-1)
    else:
        print("option not recognised!")
        sys.exit(-1)
    
def plot_recov_vs_input(recovcls, inputcls, outputFolder, display):
    """
    Plots all of the input cls compared with the recov ones
    Plots also the relative % error between them
    
    FIXME: PIXEL WINDOW FUNCTION!
    """
    
    if check_ell_range(recovcls,inputcls) == False:
        ell_max = min(recovcls['l'].max(), inputcls['l'].max())
        ell_min = max(recovcls['l'].min(), inputcls['l'].min())
        print("Using the following range: ", ell_min, ell_max)
    else:
        ell_min = recovcls['l'].min()
        ell_max = inputcls['l'].max()
    
    ell_vec = np.arange(ell_min, ell_max + 1)
    fact = ell_vec*(ell_vec + 1)
    
    # checking if pixwin function was applied:
    if search_flask_args(config, "APPLY_PIXWIN") == '1':
        import healpy as hp
        print("\nFlask simulation using pixel window function. Will apply corrections...")
        nside = get_nside(config)
        pixWin2 = (hp.pixwin(nside)[int(ell_min):(int(ell_max) + 1)])**2
    else:
        pixWin2 = np.ones_like(ell_vec)
    
    for k in recovcls.keys():
        #interpolates both functions because that's what we have:
        if k != 'l':
            splRecov = intp(recovcls['l'], recovcls[k])
            try:
                splInput = intp(inputcls['l'], inputcls[k])
                plotOk = True
            except KeyError:
                plotOk = False
            
            if plotOk:
                RecBinEll, RecBinCl, RecBinVar = bin_cls(ell_vec, splRecov(ell_vec)/pixWin2, 12)
                factBin = RecBinEll*(RecBinEll + 1)

                print("Plotting ", k, "...\n")
                fig = plt.figure()
                gs = gridspec.GridSpec(2, 1, height_ratios=[3, 1])
                plt.subplot(gs[0])
                plt.title(k + "\n(f1 = Pos; f2 = shear)")
                plt.loglog()
                plt.ylabel(r"$\ell(\ell +1)C_{\ell}$")
                plt.xlabel(r"$\ell$")
                #plt.plot(ell_vec, fact*splRecov(ell_vec)/pixWin2, label="Recov")
                plt.errorbar(RecBinEll, factBin*RecBinCl, yerr=factBin*RecBinVar, fmt='.', label="Recov ($\Delta\ell =12$)")
                plt.plot(ell_vec, fact*splInput(ell_vec), label="Input")
                plt.legend(loc=0)

                plt.subplot(gs[1])
                plt.ylabel(r"Frac. Error $\%$")
                plt.xlabel(r"$\ell$")
                #plt.xscale('log')
                plt.ylim(-15,15)

                ErrBinEll, ErrBinCl, ErrBinVar = bin_cls(ell_vec, (splInput(ell_vec) / (splRecov(ell_vec)/pixWin2) - 1)*100, 20)
                plt.errorbar(ErrBinEll, ErrBinCl, yerr=ErrBinVar, fmt='.', label="frac error")
                #plt.errorbar(RecBinEll, (splInput(RecBinEll) / RecBinCl - 1)*100, yerr=RecBinVar*100, fmt='.', label="Recov ($\Delta\ell =12$)")
                #plt.plot(ell_vec, (splInput(ell_vec) / (splRecov(ell_vec)/pixWin2) - 1)*100, label="frac error")
                plt.axhline(0, ls='--', lw=2.0)
                plt.legend(loc=0)

                figname = "/recovError-" + k + ".pdf"
                plt.savefig(outputFolder + figname, dpi=300, format="pdf", bbox_inches = 'tight')
                if display == True:
                    plt.show()
                plt.close('all')
    
    return None

def main(config, display = False):
    """
    Main part of the script
    """
    
    # uncomment the following line if you have no display access
    if display == False:
        plt.switch_backend('agg')
    else:
        None
    
    assert os.path.isfile(config), "Cannot find the config file!"
    
    # tries to create a folder in the same dir as the .config
    # if it fails, creates in the current directory
    outputFolder = create_output_folder(config) 
    
    recovClsDict = get_recov_cls_dict(config) 
    
    if checkDens2Kappa(config) == False:
        inputClsDict =  get_input_cls_dict(config, recovClsDict) 
    else:
        inputClsDict = Dens2KappaUsed(config, recovClsDict)
        
    plot_recov_vs_input(recovClsDict, inputClsDict, outputFolder, display)
    # print(recovClsDict.keys())
    # print("\n")
    # print(inputClsDict.keys())
    
    
if __name__=='__main__':
    if sys.argv[1:] == ["-h"]:
        print("USAGE: ./check_recovCls.py </path/to/flask.config> \nPlots Flask RecovCls compared to the input theory. \nPlots can be found in a new folder called 'Check_RecovCls-<configname>' in the same directory as the .config.")
        sys.exit(-1)
    else:
        config = sys.argv[1]
    main(config)
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    