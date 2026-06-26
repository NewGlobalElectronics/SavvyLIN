db File Manager
=================

![db File Manager](./images/dbManager.png)

Working with db Files
=======================

This screen allows you to load and save db files. SavvyLIN supports loading more than one db file at a time. It can even use more than one db file per bus. But, "Associated Bus" can be used to associate a given db file to only one bus. If you don't need to associate to any specific bus then set this value to -1 which means "any bus." Matching criteria is used to select J1939 or GMLAN if necessary. These two systems have special ways of interpreting the frame ID. You can create a brand new db file by clicking "Create new db" button. It will be automatically named a unique name for you. You probably don't want that name though. Any time you save a db file its name will automatically update in the list. The "Load", "Save", "Remove", "Edit" buttons are all straight forward. You can edit a db file by double clicking it in the list. 


db File Ordering
===================

The "Move Up" and "Move Down" buttons can be used to change the order of db files. Why would you care? db files are accessed in the order they are in the list. When a frame is interpreted the system goes through the db files in order. It selects the first db file that is associated to the bus the message came in on and that implements the correct message ID. So, if you have multiple db files it is possible that the order might matter. 
