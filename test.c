#include <stdio.h>
#include <termios.h>
#include <sodium.h>
#include <string.h>

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

int main(int argc, char const *argv[])
{    
    /* Hi, you can code here! */
    return 0;
}