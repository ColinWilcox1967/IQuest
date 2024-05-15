#define DOS_COPY       100
#define DOS_RENAME     101
#define DOS_DELETE     102
#define DOS_REMOVE     103
#define DOS_CREATE     104

#define DOS_UNUSED     0     /* Set When Less Than 2 Arguments Are Needed */
#define DOS_FILE       1     /* Basic Single File */
#define DOS_DIRECTORY  2     /* Directory Path */
#define DOS_WILDCARD   3     /* Multiple Entry Definition */
#define DOS_PATH_ERROR 4     /* Error In Some Part Of Path Or File Not Found */
#define FILE_ATTRIBS   ( _A_NORMAL | _A_SYSTEM | _A_HIDDEN | _A_SUBDIR )
