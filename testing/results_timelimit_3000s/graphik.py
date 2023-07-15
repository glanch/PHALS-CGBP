import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


name= "AvgSolvingTime.csv"
data=pd.read_csv(name) 
Instance_size=data["Coils"].iloc[1:5].to_numpy()

times=data["Zeit"].dropna().to_numpy()

instance4= np.zeros(5) 
instance8 = np.zeros(5) 
instance12 = np.zeros(5) 
instance20 = np.zeros(5) 

for i in range(len(times)):
    if times[i]== 'Time Limit':
        times[i]= str(0)


for i in range(5):
    instance4[i] = times[0 + i*4].replace(",", ".") 
    instance8[i] = times[1 + i*4].replace(",", ".")  
    instance12[i] = times[2 + i*4].replace(",", ".") 
    instance20[i] = times[3 + i*4].replace(",", ".") 

a=instance4.copy()
instance4[2]=a[3]
instance4[3]=a[2]
a=instance8.copy()
instance8[2]=a[3]
instance8[3]=a[2]

instance= np.vstack((instance4,instance8,instance12,instance20))

name_of_graph=["Compact\nModel", " Exakt\nFarkas\n Pricing  ", "Exakt\nkünstliche\nVariables"," Heuristik\nFarkas\n Pricing  ", "Heuristik\nkünstliche\nVariables"]

#select plot
i=2   
    

title= title_p1 + "der Instanze " + str(int(Instance_size[i]))

colors = ['red', 'blue', 'orange', "cyan", "yellow"]
fig= plt.bar(name_of_graph, instance[i], color=colors )
plt.xlabel("Methhode",fontweight='bold')
plt.ylabel("Rechenzeit in [min]",fontweight='bold')
plt.title(title, fontweight='bold')

if i > 1:
    if i==2:
        space = 0.1
    else:
        space = 1
    plt.text(4, space, 'Time Limit',rotation=90, fontsize=14.0)
    plt.text(3, space, 'Time Limit',rotation=90, fontsize=14.0)
    plt.text(2, space, 'Time Limit',rotation=90, fontsize=14.0)
    plt.text(1, space, 'Time Limit',rotation=90, fontsize=14.0)


plt.savefig(title + ".png" , dpi=400)