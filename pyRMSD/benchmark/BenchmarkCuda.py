'''
Created on 14/11/2012

@author: victor
'''
import pyRMSD.RMSDCalculator
import time
from pyRMSD.utils.proteinReading import Reader
import numpy
import sys

if __name__ == '__main__':
    using_cuda = "QCP_CUDA_CALCULATOR" in pyRMSD.RMSDCalculator.availableCalculators()
    
    if not using_cuda:
        print "Build it using --cuda."
        exit()
    
    files = ["amber_5k.pdb","amber_10k.pdb","amber_15k.pdb","amber_20k.pdb","amber_25k.pdb","amber_30k.pdb","amber_35k.pdb"]  
    for pdb_file in files:
        print "Reading file data/"+pdb_file
        sys.stdout.flush()
        
        reader = Reader("LITE_READER").readThisFile("data/"+pdb_file).gettingOnlyCAs()
        coordsets = reader.read() 
        number_of_atoms = reader.numberOfAtoms
        number_of_conformations = reader.numberOfFrames
        
        times = []
        for i in range(20):
            calculator = pyRMSD.RMSDCalculator.RMSDCalculator(coordsets, "QCP_CUDA_CALCULATOR")
            calculator.setCUDAKernelThreadsPerBlock(128, 64)
            t1 = time.time()
            rmsd = calculator.pairwiseRMSDMatrix()
            t2 = time.time()
            del rmsd
            times.append(t2-t1)
        print "With CUDA and ",pdb_file, " it took: ",numpy.mean(times),"[", numpy.std(times), "]"
    
