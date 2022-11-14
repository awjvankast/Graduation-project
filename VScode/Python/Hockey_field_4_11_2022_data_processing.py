import pandas as pd
import numpy as np

# TODO
# fix the index mathcing issues, print binned for more info

#test dataframe
a = np.array([[1,2],[3,4]])
b = pd.DataFrame(a)

#data = pd.read_csv('NoordZuidLopen.txt', sep=',', encoding = "ISO-8859-1")
data_import = pd.read_csv('NoordZuidLopen.csv', sep=';', encoding = 'ISO-8859-1',header = None)
data = data_import.fillna("")
data = data.dropna()

# Remove data line if it starts with a number but also contains non numeric characters
# Remove data if it starts with a weird character
 # loops over rows
 # Data cleaning of data entry
for k in range(len(data)):
    # Checks if the first entry is not alphabetical characters and removes whitespace for the check
    if not data.loc[k][0].replace(" ","").isalpha():
        # For all the elements in the row
        for j in range(len(data.loc[k])):
            # if the entry is not empty, check if the entry is numeric. Remove negative sign for this
            if data.loc[k][j] != "" and not data.loc[k][j].lstrip("-").isnumeric():
                #print('Deleting row' , k , 'with data: ')
                #print(data.loc[k])
                data = data.drop(k, axis = 0)
                break

data = data.reset_index()

for k in range(len(data)):
    if data.loc[k][0] == "" or data.loc[k][1] == "":
        data = data.drop(k,axis = 0)


data = data.reset_index()   

binned = pd.DataFrame(index = range(1, len(data)), columns= ['B','C','D','E','F','G'])
#binned = pd.DataFrame( columns= ['B','C','D','E','F','G'])

# Creating dataframe to concatenate:
# pd.DataFrame([1,2,3],columns = ['B'])
# Concatenating: 
# pd.concat([binned,df])

# Taking relevant data and putting it in new format
header_found = False
for k in range(len(data)):
    if data.loc[k][0].isalpha() and len(data.loc[k][0]) == 1 and header_found :
        header_found = False
        node = data[0][header_index]
        df_add = pd.DataFrame( data[header_index+1 : k][1])
        df_add = df_add.rename(columns = {1: data[0][header_index] })
        # need to change indices to packet number
        df_add.index = list(data.loc[(header_index+1) : (k-1)][0])
        indices_to_be_added = data.loc[(header_index+1) : (k-1)][0].astype(int)

        binned[node][indices_to_be_added] = df_add[node]

        #binned.iloc[indices_to_be_added][data.loc[header_index][0]] = df_add

        #binned = binned.map

        #binned = pd.concat([binned,df_add])

    if data.loc[k][0].isalpha() and len(data.loc[k][0]) == 1:
        header_index = k
        header_found = True
    
binned.to_csv('NoordZuidLopen_processed.csv',index = False)

print('end')
