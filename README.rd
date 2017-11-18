
Author: Shekhar 
Distributed Server System Example

***************** What the code will do *************************

1. Accept Multiple Clients
2. Server COnnect with appropriate time in seconds defined in DFS.conf 


**************** Functions in the Code **************************

->getFileNameFromDirectoryString function gets the fileName from the lengthy DIrectory names 
 ex : imagesfileName.txt this function gets filename fileName.txt

->show_dir_content : A recursive function to search a file of interest recursively in a given directory
 reference : https:stackoverflow.comquestions4204666how-to-list-files-in-a-directory-in-a-c-program
 path : the the path in which the search  has to be done
 fileName: the file Name to search
 fileExixts: is a global flag which will be updated respective to the existence of the file

->compare_filename_ext function : compares if the file extension exists in the ws.conf; replies -1 if not found
 filename : the filename with extension  
 replyfileType:  will be updated with the appropirate ws.conf filetype for browsers if the file exists

->readWebConfig function : parses the ws.conf file and put the extracted values to global variables
 the global variables are used to configure the webserver; thi fuction is called once after the main starts

->connection_handler function : handler for every thread createdforked
 socket_desc : is the socket ID through which the connection was established


