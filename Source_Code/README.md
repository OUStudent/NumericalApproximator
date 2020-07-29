# SOURCE CODE
The following source code is that which is used in the **Numerical Approximator** software. It is compiled into a shared library, called **NumLib.so**. 

## Compilation
The shared library is compiled through the following command:
```
gcc -g -fPIC -shared -o NumLib.so LinearAlgebra/Implementation_Files/LinearAlgebraAlgorithms.c LinearAlgebra/Implementation_Files/Matrix.c Compilation/Implementation_Files/Scanner.c Compilation/Implementation_Files/Parser.c DataStructures/Implementation_Files/SinglyLinkedList.c DataStructures/Implementation_Files/AVLTree.c DataStructures/Implementation_Files/Stack.c NumericalAnalysis/Implementation_Files/Integration.c Statistics/Implementation_Files/Distributions.c Statistics/Implementation_Files/MultivariateStatistics.c NumericalAnalysis/Implementation_Files/RootSolving.c DataStructures/Implementation_Files/DoublyLinkedList.c Interactable_Files/Implementation_Files/Normality.c Interactable_Files/Implementation_Files/Calculator.c Interactable_Files/Implementation_Files/ImportExport.c Interactable_Files/Implementation_Files/Outliers.c Interactable_Files/Implementation_Files/Analyses.c -lm
```

## Description
 All the folders, except for *Interactable_Files*, serve as the foundation of the project. All calculations, file readings/writings, I/O, etc. are computed in these files. These files were created to be extendable and reusable to an extent. The *MainApp.py* Python script only serves as the framework for user interface, as all the computations are performed in C. To be able to interact with the Python GUI script, the *Interactable_Files* was created to act as in intermediary between the raw C code and the Python script. 
