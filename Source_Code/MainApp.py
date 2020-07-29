from ctypes import *
import tkinter as tk
import tkinter.ttk as ttk
import tkinter.filedialog
import tkinter.messagebox
import tkinter.simpledialog
# import numpy as np
import tracemalloc
import os
#from TkinterDnD2 import *
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import time
plt.rcParams.update({'figure.max_open_warning': 0})
figures = []
filePath = ""
# CONSTANTS:
isFirst = 1
library = CDLL(os.getcwd()+"/CommandFiles/NumLib.so")
#library = CDLL(r"C:\Users\BScot\OneDrive\Desktop\Numerical Analysis\Here\testApp\NumLib.so")

class MATRIX(Structure):
    _fields_ = [("ncol", c_int),
                ("nrow", c_int),
                ("mat", POINTER(POINTER(c_double)))]


HEIGHT = 700
WIDTH = 1000
font1 = ('Helvetica', 12)
font2 = ('Helvetica', 12)
root = tk.Tk()
root.iconbitmap(os.getcwd()+"/CommandFiles/Icon.ico")
canvas = tk.Canvas(root, height=HEIGHT, width=WIDTH)
canvas.pack()
frame = tk.Frame(root, bg='#cccccc')  # html color picker
frame.place(relx=0, rely=0, relwidth=1, relheight=1)

# a=[[23,3]]

equations = [""]
currentIndex = 0
firstScroll = 0
def previousCommand(event):
    global currentIndex
    global firstScroll
    if firstScroll:
        currentIndex = len(equations)-2
        firstScroll = 0

    if event.delta > 0:
        entry1.delete(0, tk.END)
        entry1.insert(0, equations[currentIndex])
        currentIndex = currentIndex - 1
        if currentIndex < 0:
            currentIndex = 0
    else:
        currentIndex = currentIndex + 1
        if currentIndex > len(equations)-1:
            currentIndex = len(equations)-1
        entry1.delete(0, tk.END)
        entry1.insert(0, equations[currentIndex])

def downKey(event):
    global currentIndex
    global firstScroll
    if firstScroll:
        currentIndex = len(equations)-2
        firstScroll = 0
    currentIndex = currentIndex + 1
    if currentIndex > len(equations)-1:
        currentIndex = len(equations)-1
    entry1.delete(0, tk.END)
    entry1.insert(0, equations[currentIndex])

def upKey(event):
    global currentIndex
    global firstScroll
    if firstScroll:
        currentIndex = len(equations)-2
        firstScroll = 0
    entry1.delete(0, tk.END)
    entry1.insert(0, equations[currentIndex])
    currentIndex = currentIndex - 1
    if currentIndex < 0:
        currentIndex = 0

treeView = ttk.Treeview(root)
treeView.heading("#0", text="Variables", anchor=tk.W)
treeView.place(relx=0, rely=0.00,relwidth=0.15,relheight=1)

lowerFrame1 = tk.Frame(root, bg='#808080', bd=10)  # For entry1
lowerFrame1.place(relx=0.175, rely=0.025, relwidth=.80,relheight=0.075)

entry1 = tk.Entry(lowerFrame1, font=font1) # Main input for equations
entry1.place(relx=0.09, rely=0, relwidth=0.91, relheight=1)
entry1.bind("<MouseWheel>", previousCommand)
entry1.bind("<Up>", upKey)
entry1.bind("<Down>", downKey)

lowerFrame2 = tk.Frame(root, bg='#808080', bd=10)  # for output
lowerFrame2.place(relx=0.175, rely=0.15, relwidth=0.80, relheight=0.80)

#label3 = tk.Label(lowerFrame2, justify='left', anchor='nw', font=font1)
label3 = tk.Text(lowerFrame2, font=font1, wrap=tk.NONE)
label3.place(relx=0, rely=0, relwidth=0.95, relheight=0.95)
label3.config(state="disabled")
vbar = tk.Scrollbar(lowerFrame2, orient=tk.VERTICAL)
vbar.config(command=label3.yview)
label3.configure(yscrollcommand=vbar.set)
vbar.pack(side=tk.RIGHT, fill=tk.Y)

hbar = tk.Scrollbar(lowerFrame2, orient=tk.HORIZONTAL)
hbar.config(command=label3.xview)
label3.configure(xscrollcommand=hbar.set)
hbar.pack(side=tk.BOTTOM, fill=tk.X)

label4 = tk.Label(lowerFrame1, text="Input:", font=font2, bg='#808080')
label5 = tk.Label(root, text="Work Space:", font=font2, bg='#cccccc')
label4.place(relx=0, rely=0)
label5.place(relx=0.175, rely=0.115)

def destructor():
    for f in figures:
        plt.close(f)
    library.calculator_destructor()
    snapshot = tracemalloc.take_snapshot()
    top_stats = snapshot.statistics('lineno')
    # print("[ TOP 10 ]")
    for stat in top_stats:
        # print(stat)
        pass
    root.destroy()

def ExportVariable():
    files = [('Comma Separated File', '*.csv')]
    file = tk.filedialog.asksaveasfile(filetype=files, defaultextension=files)
    if file is None:
        return
    print(file.name)
    children = treeView.get_children()
    for child in children:
        print(treeView.item(child)["values"])

def ClearWorkSpace():

    global equations, firstScroll, currentIndex, label3
    equations = [""]
    label3.config(state="normal")
    label3.delete("1.0", tk.END)
    label3.config(state="disabled")
    firstScroll = 1
    currentIndex = 0


import pickle

class WorkSpace(object):
    def __init__(self, variables, workspaceText, equations, filePath):
        self.variables = variables
        self.workspaceText = workspaceText
        self.equations = equations
        self.filePath = filePath

def Open():
    global isFirst, equations, filePath
    question = tk.messagebox.askquestion('Open Workspace...', 'Opening another workspace will delete an unsaved workspace, are you sure you want to proceed?')
    if question != "yes":
        return

    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File")
    file = open(filename, "rb")
    obj = pickle.load(file)

    library.calculator_removeVariables()
    children = treeView.get_children()
    for child in children:
        treeView.delete(child)
    filePath = obj.filePath
    ClearWorkSpace()
    label3.config(state='normal')
    label3.insert(tk.END, obj.workspaceText)
    label3.config(state="disabled")

    equations = obj.equations
    for var in obj.variables:
        msg = str(var[0])+"="  # name
        if var[1] == 0:  #number
            msg = msg + str(var[2])
        elif var[1] == 1:  #matrix
            msg = msg +str(var[2])
            msg = msg.replace("-", "~")
        unrec = (library.calculator_scan(msg.encode("utf-8"))).decode("utf-8")
        error = library.calculator_calculate()
        treeVar = treeView.insert("", 0, text=var[0])
        isFirst = 1
        if var[1] == 0:
            value = library.calculator_assignmentValue(var[0].encode('utf-8'))
            if value == int(value):
                type = "Integer"
            else:
                type = "Float"
            treeView.insert(treeVar, "end", text="Type: "+type)
            treeView.insert(treeVar, "end", text="Value: "+str(value))
        elif var[1] == 1:
            mat = library.calculator_assignmentMatrix(var[0].encode('utf-8')).contents
            dim = str(mat.nrow) + "x" + str(mat.ncol)
            treeView.insert(treeVar, "end", text="Type: Matrix")
            treeView.insert(treeVar, "end", text="Dimensions: "+dim)

        library.calculator_ParseReturnDestructor()

def SaveHelper(file):
    global equations, filePath
    filePath = file
    if file is None:
        return
    children = treeView.get_children()
    variables = []
    for child in children:
        varName = treeView.item(child, 'text')
        unrec = (library.calculator_scan(varName.encode("utf-8"))).decode("utf-8")
        error = library.calculator_calculate()
        type = library.calculator_type()
        v = []
        v.append(varName)
        v.append(type)
        if type == 0:  # Number
            result = library.calculator_double()
            v.append(result)
            print(result)
        elif type == 1:  # Matrix
            mat = library.calculator_matrix().contents
            m = [[0 for i in range(mat.ncol)] for j in range(mat.nrow)]
            for i in range(mat.nrow):
                for j in range(mat.ncol):
                    m[i][j] = mat.mat[i][j]
            v.append(m)
        variables.append(v)
    obj = WorkSpace(variables, label3.get("1.0", tk.END), equations, filePath)
    pickle.dump(obj, open(file, "wb"))
    library.calculator_ParseReturnDestructor()
    tk.messagebox.showinfo(title="Saving Workspace", message="Successfully saved workspace...")

def SaveAs():
    files = [('Object File', '*.pickle')]
    file = tk.filedialog.asksaveasfile(initialdir=os.getcwd()+"/SavedWorkspaces", filetype=files, defaultextension=files)
    SaveHelper(file.name)
    file.close()

def Save():
    if filePath == "":
        question = tk.messagebox.askquestion('Saving Workspace...', 'The current workspace has not been saved before, would you like to save as?')
        if question != "yes":
            return
        SaveAs()
    else:
        open(filePath, "w").close()
        SaveHelper(filePath)

root.title("Numerical Approximator")
menuBar = tk.Menu(root)
fileMenu = tk.Menu(menuBar, tearoff=0)
import subprocess
def NewWorkspace():
    path = os.getcwd()+"\MainApp.exe"
    subprocess.run(path)

fileMenu.add_command(label="New Workspace", command=NewWorkspace)
fileMenu.add_command(label="Open Workspace...", command=Open)
fileMenu.add_separator()
fileMenu.add_command(label="Save Workspace", command=Save)
fileMenu.add_command(label="Save Workspace as...", command=SaveAs)
fileMenu.add_separator()
fileMenu.add_command(label="Export Variable", command=ExportVariable)
fileMenu.add_separator()
fileMenu.add_command(label="Exit", command=destructor)
menuBar.add_cascade(label="File", menu=fileMenu)

library.import_addVariable.argtypes = [c_char_p, c_char_p]
library.import_addVariable.restype = c_int

library.importReturn_destructor.argtypes = []

library.calculator_getVariableType.argtypes = [c_char_p]
library.calculator_getVariableType.restype = c_int

library.calculator_getVariableMatrix.argtypes = [c_char_p]
library.calculator_getVariableMatrix.restype = POINTER(MATRIX)

def PrintMatrix(mat, msg):
    nrow = mat.nrow
    ncol = mat.ncol
    for i in range(nrow):
        msg = msg + " [ "
        for j in range(ncol):
            if i > 20:
                msg = msg + "..., "
                if j == ncol-1:
                    msg = msg+ "] ]"
                    MsgAssigner(msg)
                    return
                continue
            if j > 20:
                msg = msg + "..."
                break
            if j is ncol-1:
                msg = msg + str(round(mat.mat[i][j],6))
            else:
                msg = msg + str(round(mat.mat[i][j],6)) + ",  "
        if i != nrow-1:
            msg = msg + " ]\n\t"
    msg = msg + '] ]'
    MsgAssigner(msg)


def ImportVariable():
    #  name = tk.askopenfilename()
    variableName = ""
    while variableName == "":
        variableName = tk.simpledialog.askstring("Import Variable", "Insert Variable Name:")

    if variableName is None:  # cancel
        return
    else:
        print(variableName)
    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File", filetypes=(("csv files", "*.csv"),("txt files", "*.txt")))
    print(filename)
    print(variableName)
    er = library.import_addVariable(filename.encode('utf-8'), variableName.encode('utf-8'))
    if er > 0:
        print("Error in file")
    else:
        children = treeView.get_children()
        for child in children:
            text = treeView.item(child, 'text')
            if text == variableName:
                treeView.delete(child)
                break
            else:
                pass
        type = library.calculator_getVariableType(variableName.encode('utf-8'))
        if type == 1:
            mat = library.calculator_getVariableMatrix(variableName.encode('utf-8')).contents
            var = treeView.insert("", 0, text=variableName)
            dim = str(mat.nrow) + "x" + str(mat.ncol)
            treeView.insert(var, "end", text="Type: Matrix")
            treeView.insert(var, "end", text="Dimensions: "+dim)
            msg = "Importing Matrix Variable: " + variableName+"\n\t["
            PrintMatrix(mat, msg)


# popup.mainloop()

library.calculator_removeVariables.argtypes = []

def ClearVariables():
    question = tk.messagebox.askquestion('Clear Variables', 'Are you sure you want to clear all current Variables? '
                                                            'This action cannot be undone.')
    if question == 'yes':
        library.calculator_removeVariables()
        children = treeView.get_children()
        for child in children:
            treeView.delete(child)

editMenu = tk.Menu(menuBar, tearoff=0)
editMenu.add_command(label="Import Variable", command=ImportVariable)
editMenu.add_separator()
editMenu.add_command(label="Clear Current Workspace", command=ClearWorkSpace)
editMenu.add_command(label="Clear Variables", command=ClearVariables)
menuBar.add_cascade(label="Edit", menu=editMenu)

library.import_matrixCSV.argtypes = [c_char_p]

library.importReturn_getMatrix.argtypes = [c_int]
library.importReturn_getMatrix.restype = POINTER(MATRIX)

library.normality_qqplotUnivariate.argtypes = [POINTER(MATRIX)]
library.normality_qqplotUnivariate.restype = POINTER(MATRIX)
library.normality_qqplotMultivariate.argtypes = [POINTER(MATRIX), POINTER(MATRIX), POINTER(MATRIX)]
library.normality_qqplotMultivariate.restype = POINTER(MATRIX)

library.outliers_findOutliers.argtypes = [POINTER(MATRIX), POINTER(MATRIX), POINTER(MATRIX), c_double]

library.outliers_returnOutliers.argtypes = []
library.outliers_returnOutliers.restype = POINTER(MATRIX)

library.outliers_getSqrdDistances.argtypes = []
library.outliers_getSqrdDistances.restype = POINTER(MATRIX)

library.outliers_getProb.argtypes = []
library.outliers_getProb.restype = c_double

library.outliers_destructor.argtypes = []

library.outliers_removeOutliers.argtypes = [POINTER(MATRIX)]
library.outliers_removeOutliers.restype = POINTER(MATRIX)

library.analyses_zscores.argtypes = [POINTER(MATRIX)]
library.analyses_zscores.restype = POINTER(MATRIX)

library.analyses_zscoresDestructor.argtypes = []

def outliersHelper(origMat, zscore):
    alpha = tk.simpledialog.askfloat("Probability Value", "Insert probability (alpha) value: ", initialvalue=0.95, minvalue=0.01,
                                         maxvalue=1)
    if alpha is None:
        return -1
    library.outliers_findOutliers(origMat, None, None, alpha)
    p = library.outliers_getProb()
    if p is None:
        return
    mat = library.outliers_getSqrdDistances()
    mat = mat.contents
    rowIndices = list(range(0,mat.nrow))
    addOn = ''
    if zscore:
        addOn = "(zscores)"
    y = []
    P = []
    for i in range(mat.nrow):
        y.append(mat.mat[i][0])
        P.append(p)
    X = np.asarray(rowIndices)
    Y = np.asarray(y)
    P = np.asarray(P)
    f = plt.figure()
    plt.scatter(X, Y)
    plt.ylabel("Squared Distances")
    plt.xlabel("Row Indices")
    plt.suptitle("Squared Distances of Data Rows "+addOn)
    plt.grid()
    plt.plot(X, P, color='r', label="Chi-Square Quantile: " + str(round(p, 3)))
    plt.legend(loc='best')
    figures.append(f)


    mat = library.outliers_returnOutliers()
    if mat:
        pass
    else:
        tk.messagebox.showinfo(title="Finding Outliers", message="No Outliers found")
        plt.show(block=False)
        return 1  # no outliers
    mat = mat.contents
    x = []
    y = []
    l = []
    for i in range(mat.nrow):
        x.append(mat.mat[i][0])
        y.append(mat.mat[i][1])
        l.append(mat.mat[i][0])
        l.append(round(mat.mat[i][1], 4))
    L = np.asarray(l)
    TwoD = np.reshape(L, (mat.nrow, 2))
    f = plt.figure()
    plt.axis('off')
    plt.axis('tight')
    df = pd.DataFrame(TwoD, columns=list(["Row Indices", "Squared Distances"]))
    plt.table(cellText=df.values, colLabels=df.columns, loc='center')
    plt.suptitle("Outliers Found: "+addOn)
    figures.append(f)
    plt.show(block=False)
    return 0

def Outliers():
    tk.messagebox.showinfo(title="Finding Outliers", message="Please select .csv or .txt file for data set")
    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File", filetypes=(("csv files", "*.csv"),("txt files", "*.txt")))
    if filename == '':
        return
    er = library.import_matrixCSV(filename.encode('utf-8'))
    if er:
        print("Error in file")
        library.importReturn_destructor()
        return
    else:
        origMat = library.importReturn_getMatrix(0)
        a = tk.messagebox.askquestion(title="Transforming Data-set", message="Use z-scores of data set in analysis (scale and center data)?")
        zscore = 0
        if a == 'yes':
            zscore = 1
            origMat = library.analyses_zscores(origMat)
        else:
            pass
        ret = outliersHelper(origMat, zscore)
        if ret != -1:
            library.outliers_destructor()
        library.analyses_zscoresDestructor()
        library.importReturn_destructor()

def normalityHelper(origMat, zscores):
    mat = library.normality_qqplotUnivariate(origMat)
    mat = mat.contents
    nrow = mat.nrow
    ncol = mat.ncol
    prob = []
    addOn = ''
    if zscores:
        addOn = " (zscores)"
    for i in range(nrow):
        prob.append(mat.mat[i][ncol-1])
    Prob = np.asarray(prob)
    plots = []
    for i in range(ncol-1, 0, -1):
        y = []
        for j in range(nrow):
            y.append(mat.mat[j][i])
        Y = np.asarray(y)
        f = plt.figure()
        plt.scatter(Prob, Y)
        m, b = np.polyfit(Prob, Y, 1)
        plt.plot(Prob, m*Prob+b)
        msg = "X"+str(i)+addOn

        plt.suptitle("QQ Plot on Variable: "+ msg)
        plt.ylabel(msg)
        plt.xlabel("Quantiles")
        plt.grid()
        #   m = np.linspace(*plt.get_xlim())
        #   plt.plot(m, m)
        figures.append(f)
        plots.append(f)

    mat = library.normality_qqplotMultivariate(origMat, None, None)
    mat = mat.contents
    x = []
    y = []
    msg = ''
    for i in range(mat.nrow):
        for j in range(mat.ncol):
            msg = msg + str(mat.mat[i][j]) + " "
        msg = msg + "\n"
    print(msg)
    for i in range(nrow):
        x.append(mat.mat[i][1])
        y.append(mat.mat[i][0])
    f = plt.figure()
    X = np.asarray(x)
    Y = np.asarray(y)
    plt.scatter(X, Y)
    m, b = np.polyfit(X, Y, 1)
    plt.plot(X, m*X+b)
    msg = "Squared Distances"+addOn
    plt.suptitle("QQ Plot on Squared Distances"+addOn)
    plt.ylabel(msg)
    plt.xlabel("Quantiles")
    plt.grid()
    figures.append(f)
    plt.show(block=False)

def OutAndNorm():
    pass

library.analyses_pca.argtypes = [POINTER(MATRIX), c_int]

library.analyses_pcaEigenValues.argtypes = []
library.analyses_pcaEigenValues.restype = POINTER(MATRIX)

library.analyses_pcaEigenVectors.argtypes = []
library.analyses_pcaEigenVectors.restype = POINTER(MATRIX)

library.analyses_pcaKthProp.argtypes = []
library.analyses_pcaKthProp.restype = POINTER(c_double)

library.analyses_pcaDestructor.argtypes = []

library.analyses_pcaScores.argtypes = [POINTER(MATRIX)]
library.analyses_pcaScores.restype = POINTER(MATRIX)

def pcaHelper(origMat, zscore):
    var = tk.IntVar()
    var.set(1)
    if zscore == 0:
        var2 = tk.IntVar()
        question = tk.Toplevel(root)
        question.title("Principal Component Analysis: Statistical Framework Type")
        question.geometry("300x100")
        tk.Label(question, text="Use Covariance or Correlation matrix in Analysis?").pack()
        tk.Radiobutton(question, text="Covariance", variable=var, value=1).pack()
        tk.Radiobutton(question, text="Correlation", variable=var, value=0).pack()
        button = tk.Button(question, text="Enter", command=lambda: [var2.set(1), question.destroy()])
        button.pack()
        button.wait_variable(var2)
        library.analyses_pca(origMat, var.get())
    else:
        library.analyses_pca(origMat, 0)  # always use cor matrix for z scores

    eigenValues = library.analyses_pcaEigenValues().contents
    eigenVectors = library.analyses_pcaEigenVectors().contents
    kthProp = library.analyses_pcaKthProp()  # size if based upon number of columns in eigenValues
    pcaScores = library.analyses_pcaScores(origMat).contents
    columns = []
    for j in range(pcaScores.ncol):
        p = []
        for i in range(pcaScores.nrow):
            p.append(pcaScores.mat[i][j])
        columns.append(p)


    Y = np.asarray(columns[1])
    i = 1
    if pcaScores.ncol != 2:
        i = 1
        for j in range(pcaScores.ncol-3):
            i = i +1
    while i >= 0:
        X = np.asarray(columns[i])
        j = pcaScores.ncol - 1

        while j > i:
            Y = np.asarray(columns[j])
            f = plt.figure()
            plt.scatter(X, Y)
            plt.xlabel("Principal Component: " + str(i+1)+" ("+str(round(kthProp[i],3))+"%)")
            plt.ylabel("Principal Component: " + str(j+1)+" ("+str(round(kthProp[j],3))+"%)")
            l = 0

            while l < pcaScores.ncol:
                plt.arrow(0, 0, eigenVectors.mat[l][i], eigenVectors.mat[l][j], color='r', alpha=0.5)
                plt.text(eigenVectors.mat[l][i]*1.15, eigenVectors.mat[l][j]*1.15, "X"+str(l+1), color='black', va='center', ha='center')
                l = l + 1

            plt.grid()
            if np.amax(Y) < 1:
                print(np.amax(Y))
                plt.ylim(top=1)
                plt.ylim(bottom=-1)
            plt.title("Principal Component Scores with Loadings")
            figures.append(f)
            j = j - 1
        i = i - 1
    plt.show(block=False)




    ncol = eigenValues.ncol
    x = list(range(1, ncol+1))
    y = []
    for i in range(ncol):
        y.append(eigenValues.mat[0][i])
    Y = np.asarray(y)
    X = np.asarray(x)

    f = plt.figure()
    plt.plot(X, Y, 'bo-', linewidth=2)
    plt.title("Scree Plot")
    plt.xlabel("Principal Components")
    plt.ylabel("Eigenvalues")
    plt.grid()
    plt.xticks(x)
    for x,y in zip(X,Y):
        label = "{:.2f}".format(y)

        plt.annotate(label, # this is the text
                     (x,y), # this is the point to label
                     textcoords="offset points", # how to position the text
                     xytext=(0,5), # distance from text to points (x,y)
                     ha='center') # horizontal alignment can be left, right or center


    figures.append(f)


    pc = np.zeros((eigenVectors.nrow, eigenVectors.ncol))
    colNames = []
    for i in range(eigenVectors.nrow):
        for j in range(eigenVectors.ncol):
            pc[i,j] = round(eigenVectors.mat[i][j],4)
            if i ==0:
                colNames.append("Component: "+str(j+1))
    f = plt.figure()
    plt.axis('off')
    plt.axis('tight')
    df = pd.DataFrame(pc, columns=colNames)
    t = plt.table(cellText=df.values, colLabels=df.columns, loc='center')
    t.set_fontsize(20)
    t.scale(1.2,1.2)
    plt.suptitle("Principal Components: ")
    figures.append(f)
    plt.show(block=False)


    cumulative = []
    prop = []
    print(kthProp)
    print(kthProp[0])
    for i in range(eigenValues.ncol):
        if i == 0:
            cumulative.append(kthProp[i])
        else:
            cumulative.append(kthProp[i]+cumulative[i-1])
        prop.append(kthProp[i])
    print(cumulative)
    print(prop)
    f = plt.figure()
    Y1 = np.asarray(prop)
    Y2 = np.asarray(cumulative)
    if zscore or var.get() == 0:
        plt.title("Correlation Explained")
    else:
        plt.title("Variance Explained")
    plt.plot(X, Y1, 'bo-', linewidth=2, label="Kth Proportion")

    plt.xticks(list(range(1,x+1)))
    plt.plot(X, Y2, 'co--', linewidth=2,label="Cumulative")
    plt.xlabel("Principal Components")
    plt.ylabel("Proportions (%)")
    plt.grid()
    plt.legend(loc='best')
    figures.append(f)
    plt.show(block=False)


    return var.get()

library.analyses_factorDestructor.argtypes = []

library.analyses_factorAnalysis.argtypes = [POINTER(MATRIX), c_int, c_int]

library.analyses_factorRotate.argtypes = [c_int, c_int]

library.analyses_factorScores.argtypes = [POINTER(MATRIX), c_int]

library.analyses_factorGetRawLoadings.argtypes = []
library.analyses_factorGetRawLoadings.restype = POINTER(MATRIX)

library.analyses_factorGetAngle.argtypes = []
library.analyses_factorGetAngle.restype = c_double

library.analyses_factorGetRotatedLoadings.argtypes = []
library.analyses_factorGetRotatedLoadings.restype = POINTER(MATRIX)

library.analyses_factorGetCommunalities.argtypes = []
library.analyses_factorGetCommunalities.restype = POINTER(MATRIX)

library.analyses_factorGetFactorScores.argtypes = []
library.analyses_factorGetFactorScores.restype = POINTER(MATRIX)

library.analyses_factorGetPsi.argtypes = []
library.analyses_factorGetPsi.restype = POINTER(MATRIX)

library.analyses_factorGetResiduals.argtypes = []
library.analyses_factorGetResiduals.restype = POINTER(MATRIX)

library.analyses_factorGetSpecificVariances.argtypes = []
library.analyses_factorGetSpecificVariances.restype = POINTER(MATRIX)

def FA():

    tk.messagebox.showinfo(title="Factor Analysis", message="Please select .csv or .txt file for data set")
    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File", filetypes=(("csv files", "*.csv"),("txt files", "*.txt")))
    if filename == '':
        return
    er = library.import_matrixCSV(filename.encode('utf-8'))
    if er:
        print("Error in file")
        library.importReturn_destructor()
        return
    else:
        origMat = library.importReturn_getMatrix(0)
        a = tk.messagebox.askquestion(title="Transforming Data-set", message="Use z-scores of data set in analysis (scale and center data)?")
        zscore = 0
        if a == 'yes':
            zscore = 1
            origMat = library.analyses_zscores(origMat)
        else:
            pass
        q = tk.messagebox.askquestion(title='Factor Analysis', message="Examine Outliers?")
        if q != 'yes':
            pass
        else:
            out = outliersHelper(origMat, zscore)
            if out == 0:
                msg = ''
                mat = origMat.contents
                print("OLD")
                for i in range(mat.nrow):
                    for j in range(mat.ncol):
                        msg = msg + str(mat.mat[i][j]) + " "
                    msg = msg + "\n"
                print(msg)
                q = tk.messagebox.askquestion(title='Factor Analysis', message="Remove Outliers?")
                if q == 'yes':
                    origMat = library.outliers_removeOutliers(origMat)
                    msg = ''
                    mat = origMat.contents
                    print("NEW")
                    for i in range(mat.nrow):
                        for j in range(mat.ncol):
                            msg = msg + str(mat.mat[i][j]) + " "
                        msg = msg + "\n"
                    print(msg)

            if out != -1:
                library.outliers_destructor()
        q = tk.messagebox.askquestion(title='Factor Analysis', message="Assess Normality?")
        if q == 'yes':
            normalityHelper(origMat, zscore)

        q = tk.messagebox.askquestion(title='Factor Analysis', message="Perform Principal Component Analysis?")
        if q != 'yes':
            var = tk.IntVar()
            var.set(1)
            if zscore == 0:
                var2 = tk.IntVar()
                question = tk.Toplevel(root)
                question.title("Factor Analysis: Statistical Framework Type")
                question.geometry("300x100")
                tk.Label(question, text="Use Covariance or Correlation matrix in Analysis?").pack()
                tk.Radiobutton(question, text="Covariance", variable=var, value=1).pack()
                tk.Radiobutton(question, text="Correlation", variable=var, value=0).pack()
                button = tk.Button(question, text="Enter", command=lambda: [var2.set(1), question.destroy()])
                button.pack()
                button.wait_variable(var2)
                library.analyses_pca(origMat, var.get())
            else:
                library.analyses_pca(origMat, 0)  # always use cor matrix for z scores
            covCorInt = var.get()
        else:
            covCorInt = pcaHelper(origMat, zscore)


        mat = origMat.contents
        factor = tk.simpledialog.askinteger("Number of Factors", "Insert number of Factors to use: ", initialvalue=2, minvalue=2,
                                         maxvalue=mat.ncol)

        if zscore:  # always use corr for zscores
            library.analyses_factorAnalysis(origMat, factor, 1)
        else:
            if covCorInt:
                covCorInt = 0
            else:
                covCorInt = 1
            library.analyses_factorAnalysis(origMat, factor, covCorInt)  # need to flip covCor because 1 is cor in FA

        loadings = library.analyses_factorGetRawLoadings().contents

        pc = np.zeros((loadings.nrow, loadings.ncol))
        colNames = []
        for i in range(loadings.nrow):
            for j in range(loadings.ncol):
                pc[i,j] = round(loadings.mat[i][j],4)
                if i ==0:
                    colNames.append("Loading: "+str(j+1))
        f = plt.figure()
        plt.axis('off')
        plt.axis('tight')
        df = pd.DataFrame(pc, columns=colNames)
        t = plt.table(cellText=df.values, colLabels=df.columns, loc='center')
        t.set_fontsize(14)
        t.scale(1.2,1.2)
        plt.suptitle("Raw non-rotated Loadings: ")
        figures.append(f)
        plt.show(block=False)

        if loadings.ncol < 4:

            while True:
                var2 = tk.IntVar()
                var = tk.IntVar()
                question = tk.Toplevel(root)
                question.title("Factor Analysis: Factor Loading Rotation through Varimax")
                if loadings.ncol == 2:
                    question.geometry("300x100")
                else:
                    question.geometry("300x125")
                tk.Label(question, text="Choose direction in which to rotate loadings:").pack()
                tk.Radiobutton(question, text="X1", variable=var, value=0).pack()
                tk.Radiobutton(question, text="X2", variable=var, value=1).pack()
                if loadings.ncol == 3:
                    tk.Radiobutton(question, text="X3", variable=var, value=2).pack()
                button = tk.Button(question, text="Enter", command=lambda: [var2.set(1), question.destroy()])
                button.pack()
                button.wait_variable(var2)
                library.analyses_factorRotate(var.get(),covCorInt)

                rotatedLoadings = library.analyses_factorGetRotatedLoadings().contents
                degree = library.analyses_factorGetAngle()
                print("Degree: "+str(degree))
                pc = np.zeros((rotatedLoadings.nrow, rotatedLoadings.ncol))
                colNames = []
                for i in range(rotatedLoadings.nrow):
                    for j in range(rotatedLoadings.ncol):
                        pc[i,j] = round(rotatedLoadings.mat[i][j],4)
                        if i == 0:
                            colNames.append("Loading: "+str(j+1))
                f = plt.figure()
                plt.axis('off')
                plt.axis('tight')
                df = pd.DataFrame(pc, columns=colNames)
                t = plt.table(cellText=df.values, colLabels=df.columns, loc='center')
                t.set_fontsize(14)
                t.scale(1.2,1.2)
                if var.get() == 0:
                    plt.suptitle("Rotated Loadings in direction of X1 through angle: " +str(round(degree, 3)))
                elif var.get() == 1:
                    plt.suptitle("Rotated Loadings in direction of X2 through angle: " +str(round(degree, 3)))
                elif var.get() == 2:
                    plt.suptitle("Rotated Loadings in direction of X3 through angle: " +str(round(degree, 3)))
                figures.append(f)
                plt.show(block=False)
                question = tk.messagebox.askquestion('Rotate Loadings', 'Rotate Loadings through a different angle?')
                if question != 'yes':
                    break

        else:  # cannot rotated more than three loadings
            library.analyses_factorRotate(0, 0)  # just sets rotated loadings to copy of raw loadings
            degree = "NA"
            rotatedLoadings = loadings

        kthProp = library.analyses_pcaKthProp()  # size if based upon number of columns in eigenValues
        sum = 0
        for i in range(factor):
            sum = sum + kthProp[i]

        # factor report
        factorReport = tk.Toplevel(root)
        factorReport.title("Factor Analysis Full Report")
        factorReport.geometry("900x400")
        frame = tk.Frame(factorReport, bg='#808080')  # html color picker
        frame.place(relx=0, rely=0, relwidth=1, relheight=1)
        frame1 = tk.Frame(factorReport, bg='#cccccc', bd=10)  # For title
        frame1.place(relx=0.025, rely=0.025, relwidth=.95, relheight=0.175)
        titleFont = ('Helvetica', 20)
        subTitleFont = ('Helvetica', 13)
        label1 = tk.Label(frame1, text="Factor Analysis Full Report: m="+str(factor)+" (" + str(round(sum, 2))+"%)", font=titleFont, bg='#cccccc')
        label1.pack()
        frame2 = tk.Frame(factorReport, bg='#cccccc', bd=10)  # For loadings
        frame2.place(relx=0.025, rely=0.23, relwidth=0.53, relheight=0.125)

        # LOADING FACTORS
        frame1 = tk.Frame(factorReport, bg='#cccccc', bd=10)
        frame1.place(relx=0.025, rely=0.4, relwidth=0.53, relheight=0.55)
        labelScroll1 = tk.Text(frame1, relief='flat')
        labelScroll1.place(relx=0, rely=0, relwidth=1, relheight=1)
        labelF1 = tk.Text(labelScroll1, font=font1, relief='flat')
        labelScroll1.window_create('insert', window=labelF1)
        #hbar.place(relx=0.05, rely=0.12, relheight=0.3)


        # COMMUNALITIES
        frame3 = tk.Frame(factorReport,  bg='#cccccc', bd=10)  # For Communalitites
        frame3.place(relx=0.585, rely=0.23, relwidth=0.18, relheight=0.125)
        frameComm = tk.Frame(factorReport, bg='#cccccc', bd=10)
        frameComm.place(relx=0.585, rely=0.4, relwidth=0.18, relheight=0.55)
        labelScroll2 = tk.Text(frameComm, relief='flat')
        labelScroll2.place(relx=0,rely=0,relwidth=1,relheight=1)
        labelF3 = tk.Text(labelScroll2, bg='white', font=font1)
        labelScroll2.window_create('insert', window=labelF3)


        # SPECIFIC VARIANCES
        frame4 = tk.Frame(factorReport,  bg='#cccccc', bd=10)  # For Specicif Variances
        frame4.place(relx=0.795, rely=0.23, relwidth=0.18, relheight=0.125)
        frameVar = tk.Frame(factorReport, bg='#cccccc', bd=10)
        frameVar.place(relx=0.795, rely=0.4, relwidth=0.18, relheight=0.55)
        labelScroll3 = tk.Text(frameVar, relief='flat')
        labelScroll3.place(relx=0,rely=0,relwidth=1,relheight=1)
        labelF4 = tk.Text(labelScroll3, bg='white', font=font1)
        labelScroll3.window_create('insert', window=labelF4)

        label2 = tk.Label(frame2, text="Estimated Factor Loadings (Rotated through angle: "+str(degree) +")", font=subTitleFont, bg='#cccccc')
        label2.pack()
        label3 = tk.Label(frame3, text="Communalities", font=subTitleFont, bg='#cccccc')
        label3.pack()
        label4 = tk.Label(frame4, text="Specific Variances", font=subTitleFont, bg='#cccccc')
        label4.pack()

        # FACTOR LOADINGS
        for i in range(rotatedLoadings.nrow+1):
            for j in range(rotatedLoadings.ncol+1):
                if i == 0: # first row, so F1, F2, ...
                    if j == 0:  # first empty cell
                        e = tk.Entry(labelF1, width=4, font=('Helvetica', 12))
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "")
                    else:
                        e = tk.Entry(labelF1, width=8, font=('Helvetica', 12),justify='center')
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "F"+str(j))
                elif j == 0:  # not first row but first column, so X1, X2, ...
                    e = tk.Entry(labelF1, width=4, font=('Helvetica', 12))
                    e.grid(row=i, column=j)
                    e.insert(tk.END, "X"+str(i))
                else:  # the actual numbers of the table
                    e = tk.Entry(labelF1, width=8, font=('Helvetica', 12),justify='right')
                    e.grid(row=i, column=j)
                    e.insert(tk.END, round(float(rotatedLoadings.mat[i-1][j-1]), 4))
                e.configure(state='readonly')
                labelF1.window_create(tk.INSERT, window=e)
            labelF1.insert('end', '\n')


        labelF1.configure(xscrollcommand=hbar.set)
        vbar = tk.Scrollbar(labelScroll1, orient=tk.VERTICAL)
        vbar.configure(command=labelF1.yview)
        labelF1.configure(yscrollcommand=vbar.set)
        vbar.pack(side=tk.RIGHT, fill=tk.Y)
        labelF1.pack(fill='both')
        labelF1.configure(state='disabled')
        labelScroll1.configure(state='disabled')


        # COMMUNALITIES
        communalities = library.analyses_factorGetCommunalities().contents
        for i in range(communalities.nrow+1):
            for j in range(communalities.ncol+1):
                if i == 0:  # first row, so F1 through F4
                    if j == 0:  # first empty cell
                        e = tk.Entry(labelF3, width=4, font=('Helvetica', 12))
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "")
                    else:
                        e = tk.Entry(labelF3, width=8, font=('Helvetica', 12),justify='center')
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "H")

                elif j == 0:  # not first row but first column, so X1 through X*
                    e = tk.Entry(labelF3, width=4, font=('Helvetica', 12))
                    e.grid(row=i, column=j)
                    e.insert(tk.END, "X"+str(i))
                else:  # the actual numbers of the table
                    e = tk.Entry(labelF3, width=8, font=('Helvetica', 12),justify='right')
                    e.grid(row=i, column=j)
                    e.insert(tk.END, round(float(communalities.mat[i-1][j-1]), 4))
                e.configure(state='readonly')
                labelF3.window_create(tk.INSERT, window=e)
            labelF3.insert('end', '\n')

        labelF3.configure(xscrollcommand=hbar.set)
        vbar = tk.Scrollbar(labelScroll2, orient=tk.VERTICAL)
        vbar.configure(command=labelF3.yview)
        labelF3.configure(yscrollcommand=vbar.set)
        vbar.pack(side=tk.RIGHT, fill=tk.Y)
        labelF3.pack(fill='both')
        labelF3.configure(state='disabled')
        labelScroll2.configure(state='disabled')


        # SPECIFIC VARAINCES
        matrix = np.array([["", "Lambda"],
                           ["X1", 3],
                           ["X2", 42],
                           ["X3", 2.3323],
                           ["X4", .2342],
                           ["X5", 3232],
                           ["X6", 2422],
                           ["X7", 2342],
                           ["X8", .2422],
                           ["X9", .2422]])

        specificVar = library.analyses_factorGetSpecificVariances().contents
        for i in range(specificVar.nrow+1):
            for j in range(specificVar.ncol+1):
                if i == 0:  # first row, so F1 through F4
                    if j == 0:  # first empty cell
                        e = tk.Entry(labelF4, width=4, font=('Helvetica', 12))
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "")
                    else:
                        e = tk.Entry(labelF4, width=8, font=('Helvetica', 12), justify='center')
                        e.grid(row=i, column=j)
                        e.insert(tk.END, "\u03bb")

                elif j == 0:  # not first row but first column, so X1 through X*
                    e = tk.Entry(labelF4, width=4, font=('Helvetica', 12))
                    e.grid(row=i, column=j)
                    e.insert(tk.END, "X"+str(i))
                else:  # the actual numbers of the table
                    e = tk.Entry(labelF4, width=8, font=('Helvetica', 12),justify='right')
                    e.grid(row=i, column=j)
                    e.insert(tk.END, round(float(specificVar.mat[i-1][j-1]), 4))
                e.configure(state='readonly')
                labelF4.window_create(tk.INSERT, window=e)
            labelF4.insert('end', '\n')

        labelF4.configure(xscrollcommand=hbar.set)
        vbar = tk.Scrollbar(labelScroll3, orient=tk.VERTICAL)
        vbar.configure(command=labelF4.yview)
        labelF4.configure(yscrollcommand=vbar.set)
        vbar.pack(side=tk.RIGHT, fill=tk.Y)
        labelF4.pack(fill='both')
        labelF4.configure(state='disabled')
        labelScroll3.configure(state='disabled')

        factorReport.resizable(tk.FALSE, tk.FALSE)
        print("Zscore flag: "+str(zscore))
        library.analyses_factorScores(origMat, zscore)

        # rotated loadings
        columns = []
        for j in range(rotatedLoadings.ncol):
            p = []
            for i in range( rotatedLoadings.nrow):
                p.append(rotatedLoadings.mat[i][j])
            columns.append(p)

        i = 1
        if rotatedLoadings.ncol != 2:
            i = 1
            for j in range(rotatedLoadings.ncol-3):
                i = i +1
        while i >= 0:
            X = np.asarray(columns[i])
            j = rotatedLoadings.ncol - 1
            while j > i:
                Y = np.asarray(columns[j])
                f = plt.figure()
                plt.scatter(X,Y)
                plt.xlabel("Factor "+str(i+1))
                plt.ylabel("Factor "+str(j+1))
                plt.grid()
                iter = 1
                for x,y in zip(X,Y):
                    label = "X"+str(iter)
                    iter = iter + 1
                    plt.annotate(label, # this is the text
                                 (x,y), # this is the point to label
                                 textcoords="offset points", # how to position the text
                                 xytext=(0,5), # distance from text to points (x,y)
                                 ha='center') # horizontal alignment can be left, right or center

                plt.title("Factor Loadings")
                figures.append(f)
                j = j - 1
            i = i - 1
        plt.show(block=False)
        question = tk.messagebox.askquestion('Factor Loadings', 'Would you like to export the rotated factor loadings from this analysis?')
        if question == "yes":
            files = [('Comma Separated File', '*.csv')]
            file = tk.filedialog.asksaveasfile(filetype=files, defaultextension=files)
            if file is None:
                return
            else:
                print(file)
                for i in range(rotatedLoadings.nrow):
                    for j in range(rotatedLoadings.ncol):
                        file.write(str(rotatedLoadings.mat[i][j]))
                        if j != rotatedLoadings.ncol-1:
                            file.write(",")
                    file.write("\n")
                file.close()
        scores = library.analyses_factorGetFactorScores().contents
        columns = []
        for j in range(scores.ncol):
            p = []
            for i in range( scores.nrow):
                p.append(scores.mat[i][j])
            columns.append(p)

        i = 0
        # ncol = 2
        # i = 0: j = 1, j = 2
        # i = 1: j = 2
        # ncol = 3
        # i = 1: j = 2
        # i = 0: j = 1, j=2
        i = 1
        if scores.ncol != 2:
            i = 1
            for j in range(scores.ncol-3):
                i = i +1
        # ncol = 5 => 3
        # ncol = 4 => 2
        # ncol = 3 => 1
        # ncol = 2 => 1
        while i >= 0:
            X = np.asarray(columns[i])
            j = scores.ncol-1
            while j > i:

                Y = np.asarray(columns[j])
                f = plt.figure()
                plt.scatter(X,Y)
                plt.xlabel("Factor "+str(i+1))
                plt.ylabel("Factor "+str(j+1))
                plt.grid()
                plt.title("Factor Scores")
                figures.append(f)
                j = j - 1
            i = i - 1
        plt.show(block=False)

        question = tk.messagebox.askquestion('Factor Scores', 'Would you like to export the factor scores from this analysis?')
        if question == "yes":
            files = [('Comma Separated File', '*.csv')]
            file = tk.filedialog.asksaveasfile(filetype=files, defaultextension=files)
            if file is None:
                return
            else:
                print(file)
                for i in range(scores.nrow):
                    for j in range(scores.ncol):
                        file.write(str(scores.mat[i][j]))
                        if j != scores.ncol-1:
                            file.write(",")
                    file.write("\n")
                file.close()

        library.analyses_pcaDestructor()  # This calls library.analyses_zscoresDestructor()
        library.analyses_factorDestructor()
        library.importReturn_destructor()


def PCA():

    tk.messagebox.showinfo(title="Principal Component Analysis", message="Please select .csv or .txt file for data set")
    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File", filetypes=(("csv files", "*.csv"),("txt files", "*.txt")))
    if filename == '':
        return
    er = library.import_matrixCSV(filename.encode('utf-8'))
    if er:
        print("Error in file")
        library.importReturn_destructor()
        return
    else:
        origMat = library.importReturn_getMatrix(0)
        a = tk.messagebox.askquestion(title="Transforming Data-set", message="Use z-scores of data set in analysis (scale and center data)?")
        zscore = 0
        if a == 'yes':
            zscore = 1
            origMat = library.analyses_zscores(origMat)
        else:
            pass
        q = tk.messagebox.askquestion(title='Principal Component Analysis', message="Examine Outliers?")
        if q != 'yes':
            pass
        else:
            out = outliersHelper(origMat, zscore)
            if out == 0:
                msg = ''
                mat = origMat.contents
                print("OLD")
                for i in range(mat.nrow):
                    for j in range(mat.ncol):
                        msg = msg + str(mat.mat[i][j]) + " "
                    msg = msg + "\n"
                print(msg)
                q = tk.messagebox.askquestion(title='Principal Component Analysis', message="Remove Outliers?")
                if q == 'yes':
                    origMat = library.outliers_removeOutliers(origMat)
                    msg = ''
                    mat = origMat.contents
                    print("NEW")
                    for i in range(mat.nrow):
                        for j in range(mat.ncol):
                            msg = msg + str(mat.mat[i][j]) + " "
                        msg = msg + "\n"
                    print(msg)

            if out != -1:
                library.outliers_destructor()
        q = tk.messagebox.askquestion(title='Principal Component Analysis', message="Assess Normality?")
        if q == 'yes':
            normalityHelper(origMat, zscore)
        tk.messagebox.showinfo(title="Principal Component Analysis",
                               message="Now Performing Principal Component Analysis")
        pcaHelper(origMat, zscore)

        library.analyses_pcaDestructor()  # This calls library.analyses_zscoresDestructor()
        library.importReturn_destructor()

def Normality():
    tk.messagebox.showinfo(title="Checking Normality", message="Please select .csv or .txt file for data set")
    filename = tk.filedialog.askopenfilename(initialdir=os.getcwd(), title="Select a File", filetypes=(("csv files", "*.csv"),("txt files", "*.txt")))
    if filename == '':
        return
    er = library.import_matrixCSV(filename.encode('utf-8'))
    if er:
        print("Error in file")
        library.importReturn_destructor()
        return
    else:
        origMat = library.importReturn_getMatrix(0)
        a = tk.messagebox.askquestion(title="Transforming Data-set", message="Use z-scores of data set in analysis (scale and center data)?")
        zscore = 0
        if a == 'yes':
            zscore = 1
            origMat = library.analyses_zscores(origMat)
            msg = ''
            print("HERE!!!!")
            t = origMat.contents
            for i in range(t.nrow):
                for j in range(t.ncol):
                    msg = msg + str(t.mat[i][j]) + ', '
                msg = msg + '\n'
            print(msg)
            print("END!!!")
        else:
            pass
        normalityHelper(origMat, zscore)
        library.analyses_zscoresDestructor()
        library.importReturn_destructor()


analysisMenu = tk.Menu(menuBar, tearoff=0)
analysisMenu.add_command(label="Check for Outliers", command=Outliers)
analysisMenu.add_command(label="Assess Normality", command=Normality)
analysisMenu.add_separator()
analysisMenu.add_command(label="Multivariate Regression")
analysisMenu.add_separator()
analysisMenu.add_command(label="Principal Component Analysis", command=PCA)
analysisMenu.add_command(label="Factor Analysis", command=FA)
analysisMenu.add_command(label="Canonical Correlation Analysis")
analysisMenu.add_separator()
analysisMenu.add_command(label="Discrimination and Classification")
analysisMenu.add_command(label="Cluster Analysis")
menuBar.add_cascade(label="Analysis", menu=analysisMenu)

import webbrowser as wb

def HelpFile():
    wb.open_new(os.getcwd() + "/CommandFiles/HelpFile.pdf")

helpMenu = tk.Menu(menuBar, tearoff=0)
helpMenu.add_command(label="Help File", command=HelpFile)
helpMenu.add_command(label="Documentation")
menuBar.add_cascade(label="Help", menu=helpMenu)

root.config(menu=menuBar)

# initialize
library.calculator_initialize.argtypes = []
library.calculator_initialize()

# scan
library.calculator_scan.argtypes = [c_char_p]
library.calculator_scan.restype = c_char

# calculate
library.calculator_calculate.argtypes = []
library.calculator_calculate.restype = c_int

# type
library.calculator_type.argtypes = []
library.calculator_type.restype = c_int

# double
library.calculator_double.argtypes = []
library.calculator_double.restype = c_double

# scanner destructor
library.calculator_scannerDestructor.argtypes = []

# parseReturn Destructor
library.calculator_ParseReturnDestructor.argtypes = []

# calculator destructor
library.calculator_destructor.argtypes = []

# calculator unrecVar
library.calculator_unrecVar.argtypes = []
library.calculator_unrecVar.restype = c_char_p

# calculator unrecToken
library.calculator_unrecToken.argtypes = []
library.calculator_unrecToken.restype = c_char

# calculator unrecVarDestructor
library.calculator_unrecVarDestructor.argtypes = []

# calculator method
library.calculator_method.argtypes = []

# calculator assignment type
library.calculator_assignmentType.argytpes = [c_char_p]
library.calculator_assignmentType.restype = c_int

# calculator assignment type
library.calculator_assignmentValue.argytpes = [c_char_p]
library.calculator_assignmentValue.restype = c_double

# calcultor matrix
library.calculator_matrix.argtypes = []
library.calculator_matrix.restype = POINTER(MATRIX)

# calculator methodHolder top
library.methodHolder_getTop.argytpes = []
library.methodHolder_getTop.restype = POINTER(MATRIX)

# calculator methodHolder bottom
library.methodHolder_getBottom.argytpes = []
library.methodHolder_getBottom.restype = POINTER(MATRIX)

# calculator methodHolder destructor
library.methodHolderDestructor.argytpes = []

# calculator assignment type
library.calculator_assignmentMatrix.argytpes = [c_char_p]
library.calculator_assignmentMatrix.restype = POINTER(MATRIX)


myErrors = {1: "Error: Semantic Error in Equation",
            2: "Error: Missing Left Parenthesis '('",
            3: "Error: Missing Right Parenthesis ')'",
            4: "Error: Missing Left Bracket '['",
            5: "Error: Missing Right Bracket ']'",
            6: "Error: Missing Comma ','",
            7: "Error: Missing Obtain Symbol '$'",
            8: "Error: Expecting Floating Point value in an operation",
            9: "Error: Expecting Matrix value in an operation",
            10: "Error: Expecting Boolean value in an operation",
            11: "Error: The Derivative option in newton() must be NULL",
            12: "Error: Unrecognized Token: ",
            13: "Error: bisection() method requires a Change of signs over the given interval",
            14: "Error: Expecting Variable value in a function",
            15: "Error: Failed Eigen function, given Matrix must be symmetric",
            16: "Error: Expecting 'values' or 'vectors' for Eigen function obtain value",
            17: "Error: Expecting 'Q' or 'R' for QR function obtain value",
            18: "Error: Expecting 'L' or 'U' for LU function obtain value",
            19: "Error: Failed LU function, given Matrix must be square",
            20: "Error: Expecting obtain values for function",
            21: "Error: Given probability for qhcisq() and qnorm() must be between 0 and 1",
            22: "Error: Given index for getCol() is out of bounds for given matrix",
            23: "Error: Given index for getRow() is out of bounds for given matrix",
            24: "Error: Failed houseHolder() function, given matrix must be symmetric",
            25: "Error: Failed naiveGauss() function, first matrix must be nx(n+1) if second argument is NULL; "
                "else, first matrix must be square",
            26: "Error: Failed to add matrices, must have same dimensions",
            27: "Error: Failed to minus matrices, must have same dimensions",
            28: "Error: Failed to multiply matrices, must have compatable dimensions",
            29: "Error: Not allowed to exponentiate '^' matrices",
            30: "Error: Not allowed to divide '/' matrices",
            31: "Error: Not allowed to minus '-' constant by a matrix",
            32: "Error: Not allowed to divide '/' constant by a matrix",
            33: "Error: Not allowed to exponentiate '^' by a matrix",
            34: "Error: Not allowed to negate non Floating Point or Matrix values",
            35: "Error: Expecting Floating Point or Matrix value in an operation",
            36: "Error: Given index for matrix is out of bounds",
            37: "Error: Unrecognized Variable Name: ",
            38: "Error: Expecting Integer value in an operation",
            39: "Error: Semantic Error in creating Matrix",
            40: "Error: Unequal matrix dimensions in creation",
            41: "Error: Failed backSub() function, given matrix must be upper triangular with non-zero values on "
                "the diagonal",
            42: "Error: Cannot divide constant by zero value",
            43: "Error: Cannot take natural logarithmic function of zero value",
            44: "Error: The letter 'e' has been reserved for Euler's Constant: 2.718...",
            45: "Error: Standard Deviation for p and q distribution functions cannot be zero"
            }

def Errors(error):
    msg = myErrors[error]
    if error == 37:
        temp = library.calculator_unrecVar()
        msg = msg + temp.decode("utf-8")
        library.calculator_unrecVarDestructor()
        plt.close('all')
        # print(msg)
    return msg



def MsgAssigner(msg):
    global isFirst
    if isFirst:
        # label3.configure(text=msg)
        label3.config(state='normal')
        label3.insert(tk.END, msg)
        isFirst = 0
        label3.config(state="disabled")
    else:
        prev = label3.get("1.0", tk.END)
        label3.config(state='normal')
        label3.delete("1.0", tk.END)
        new = msg + "\n\n" + str(prev)+'\n'
        #label3.configure(text=new)
        label3.insert("1.0", new)
        label3.config(state="disabled")

def TypeName(type):
    if type == 0:
        return "Number"
    elif type == 1:
        return "Matrix"

tracemalloc.start()
def Test(event):
    global firstScroll
    eq1 = entry1.get()
    eq2 = eq1.encode('utf-8')
    msg = "Calculating: " + eq1 + "\n\t"
    unrec = (library.calculator_scan(eq2)).decode("utf-8")
    if unrec != '\0':
        msg = msg + "Error: Unrecognized symbol '" + unrec+ "'"
        MsgAssigner(msg)
        library.calculator_scannerDestructor()
        return

    error = library.calculator_calculate()
    if error > 0:
        msg = msg + Errors(error)
        MsgAssigner(msg)

        return

    # calcualtor_method LU, QR, Eigen
    library.calculator_method()
    type = library.calculator_type()
    entry1.delete(0,tk.END)
    entry1.insert(0,"")
    if type == -1:  # Assignment
        msg = "Assigning: " + eq1
        name = eq1.split('=')[0]
        name.strip()
        name.replace(" ", "")
        name = ''.join(name.split())
        type = library.calculator_getVariableType(name.encode('utf-8'))
       # calculator_getVariableType
        children = treeView.get_children()
        for child in children:
            text = treeView.item(child, 'text')
            if text == name:
                treeView.delete(child)
                break
            else:
                pass

        var = treeView.insert("", 0, text=name)
        #  var.column("#0", minwidth=0, width=100, strecth=NO)
        type = TypeName(type)
        if type == "Number":
            value = library.calculator_assignmentValue(name.encode('utf-8'))
            if value == int(value):
                type = "Integer"
            else:
                type = "Float"
            treeView.insert(var, "end", text="Type: "+type)
            treeView.insert(var, "end", text="Value: "+str(value))
            msg = msg + "\n\t" + str(value)
        elif type == "Matrix":
            mat = library.calculator_assignmentMatrix(name.encode('utf-8')).contents
            dim = str(mat.nrow) + "x" + str(mat.ncol)
            treeView.insert(var, "end", text="Type: "+type)
            treeView.insert(var, "end", text="Dimensions: "+dim)
            msg = msg + "\n\t["
            nrow = mat.nrow
            ncol = mat.ncol
            for i in range(nrow):
                msg = msg + " [ "
                for j in range(ncol):
                    if j is ncol-1:
                        msg = msg + str(round(mat.mat[i][j],6))
                    else:
                        msg = msg + str(round(mat.mat[i][j],6)) + ",  "
                if i != nrow-1:
                    msg = msg + " ]\n\t"
            msg = msg + '] ]'
        else:
            pass

        MsgAssigner(msg)

    elif type == 0:  # Number
        result = library.calculator_double()
        msg = msg + str(result)
        MsgAssigner(msg)
    elif type == 1:  # Matrix
        mat = library.calculator_matrix().contents
        msg = msg + "["
        PrintMatrix(mat, msg)

    elif type == 2:
        bool = int(library.calculator_double())
        if bool == -1:
            msg = msg + "ERROR: The request you have made as failed"
        elif bool == 0:
            msg = msg + "False"
        elif bool == 1:
            msg = msg + "True"
        elif bool == 2:
            msg = msg + "Success"
        MsgAssigner(msg)
    elif type == 3:
        msg = msg + "L:\n\t["
        mat = library.methodHolder_getTop().contents
        nrow = mat.nrow
        ncol = mat.ncol
        for i in range(nrow):
            msg = msg + " [ "
            for j in range(ncol):
                if j is ncol-1:
                    msg = msg + str(round(mat.mat[i][j],6))
                else:
                    msg = msg + str(round(mat.mat[i][j],6)) + ",  "
            if i != nrow-1:
                msg = msg + " ]\n\t"
        msg = msg + '] ]'

        msg = msg + "\n\n\tU:\n\t["
        mat = library.methodHolder_getBottom().contents
        PrintMatrix(mat, msg)

        library.methodHolderDestructor()
    elif type == 4:
        msg = msg + "Q:\n\t["
        mat = library.methodHolder_getTop().contents
        nrow = mat.nrow
        ncol = mat.ncol
        for i in range(nrow):
            msg = msg + " [ "
            for j in range(ncol):
                if j is ncol-1:
                    msg = msg + str(round(mat.mat[i][j],6))
                else:
                    msg = msg + str(round(mat.mat[i][j],6)) + ",  "
            if i != nrow-1:
                msg = msg + " ]\n\t"
        msg = msg + '] ]'

        msg = msg + "\n\n\tR:\n\t["
        mat = library.methodHolder_getBottom().contents
        PrintMatrix(mat, msg)
    elif type == 5:
        msg = msg + "Values:\n\t["
        mat = library.methodHolder_getTop().contents
        nrow = mat.nrow
        ncol = mat.ncol
        for i in range(nrow):
            msg = msg + " [ "
            for j in range(ncol):
                if j is ncol-1:
                    msg = msg + str(round(mat.mat[i][j],6))
                else:
                    msg = msg + str(round(mat.mat[i][j],6)) + ",  "
            if i != nrow-1:
                msg = msg + " ]\n\t"
        msg = msg + '] ]'

        msg = msg + "\n\n\tVectors:\n\t["
        mat = library.methodHolder_getBottom().contents
        PrintMatrix(mat, msg)
    else:
        print("Return Value: " + str(type))
    firstScroll = 1
    equations.insert(len(equations)-1, eq1)
    library.calculator_ParseReturnDestructor()



entry1.bind("<Return>", Test)




root.protocol("WM_DELETE_WINDOW", destructor)

root.mainloop()