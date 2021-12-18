/*
    Pretty amazing password manager written in C

    1. Tasks

        Authentication

        - get the password
        - verify the password (login) or validate input (signin)
        - grant or not the access
        
        Execution

        - show passwords (by user)
        - add password (by user)
        - remove password (by user)
        - modify password (by user)
        - retrieve password (by user)
        - encrypting/decrypting passwords
        - saving passwords in the clipboard
        - clearing the clipboard after a defined amount of time
        - keeping all buffers clean

        Closure

        - clear everything (buffers, clipboard)

    2. Commands

        - show(char **args): it shows all accounts with hidden passwords (for security 
            purposes), because its task is to make the user see what are his accounts; 
            if the user wants to retrieve a password he will have to call the get() 
            function. As well as all other commands it accepts multiple args, but 
            those will be ignored because useless (it will run as a void function).

        - add(char **args): it adds a new account to files. It accepts account_name,
            user_or_mail and password as args.
        
        - remove(char **args): it removes an account from files. It accepts
            account_name (which is unique) as single arg.

        - modify(char **args): it modifyies user_or_mail or/and password of an account. 
            Accepted args are not yet decided 'cause this function is a bit messy. 
            Maybe it will consist in some -something -> -p for pass and -u for user.

        - get(char **args): it saves the specified account in the user's clipboard,
            so that the password never gets visible (inputs will have echo disabled).
            It accepts account_name as single arg.

    3. Storage method

        - account_name
        - user_or_mail
        - password 

        all of these may be put in two files, one for keeping accounts' and users'
        names, the other for keeping passwords (/etc/passwd and /etc/shadow style). 
        This because if the user calls show() with all data saved in the same file, 
        it will be decrypted all account_name, user_or_name and password, putting 
        the latter at risk uselessly 'cause it will not be used to produce the output.
*/

/*  password input
    password conversion to key1 in secure memory
    password conversion to key2 in secure memory

    key derivation

    the idea is to derive from the password hash two subkeys that will be the actual encryption keys.

    1. hash = hash(password)
    2. if hash == stored_hash
        login passed
       else
        login not passed
    3. subkey1 = subkey(hash), subkey2 = subkey(hash)

    show

    it shows all accounts in the database with hidden passwords

    - decrypt file1 with key1 in an array
    - print out the array

    add

    it adds account name, user and password to the database

    - get input
    - verify input
    - decrypt file1 with key1
    - append account name and user to file1
    - encrypt file1 with key1
    - decrypt file2 with key2
    - append password to file2
    - encrypt file2 with key2

    remove

    it removes account name, user and password from the database

    - get input
    - verify input
    - decrypt file1 with key1
    - if account is present remove it, else leave file1 unmodified and exit
    - encrypt file1 with key1
    - decrypt file2 with key2
    - remove key at account's index
    - recrypt file2 with key2

    modify

    it permits you to modify only user and password for secure key management purposes,
    it is not meant to be used to fix typing errors or similar

    - get input
    - verify input
    - decrypt file1 with key1
    IF IT IS WANTED TO MODIFY BOTH USER AND PASSWORD
    - if account is present modify user, else leave file1 unmodified and exit
    - encrypt file1 with key1
    - decrypt file2 with key2
    - replace password at same account index with the new one
    - encrypt file2 with key2
    IF IT IS WANTED TO MODIFY ONLY USER
    - if account is present modify user, else leave file1 unmodified and exit
    - encrypt file1 with key1
    IF IT IS WANTED TO MODIFY ONLY PASSWORD
    - if account is present get its index, else leave file1 unmodified and exit
    - decrypt file2 with key2
    - replace password at the account index with the new one
    - encrypt file2 with key2

    get

    it saves the password in the user's clipboard and after a while it clears the clipboard up

    - get input
    - verify input
    - decrypt file1 with key1
    - if account do exists get its index, else leave file1 unmodified and exit
    - decrypt file2 with key2
    - save password at account's index in the clipboard
    - start a timer
    - encrypt file2 wiht key2
    - when the timer runs out clear the clipboard

    fixes

    - verify that both files have the same number of lines
    - verify that indexes correspond or make it impossible to corrupt account-password relation
*/