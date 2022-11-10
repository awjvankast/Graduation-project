import pandas as pd
import numpy as np

# TODO
# add data cleaning of header entry's
# Format new dataframe so all the packets match and nodenames are columns

#test dataframe
a = np.array([[1,2],[3,4]])
b = pd.DataFrame(a)

#data = pd.read_csv('NoordZuidLopen.txt', sep=',', encoding = "ISO-8859-1")
data_import = pd.read_csv('NoordZuidLopen.csv', sep=';', encoding = 'ISO-8859-1',header = None)
data = data_import.fillna("")

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
                print('Deleting row' , k , 'with data: ')
                print(data.loc[k])
                data = data.drop(k, axis = 0)
                break



if data.loc[0][0] == "A":
    print(data.loc[0])
    
    #loops over columns
for a in data:
    print(a)


print('end')
