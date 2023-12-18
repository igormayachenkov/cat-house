exports.tables = {
    // WS CONFIG TABLES
    'workspaces'   : {
        fields:{
            id:       'VARCHAR(32) NOT NULL',
            name:     'VARCHAR(255) NOT NULL'
        }, 
        primary_key: 'id'
    },
    'modules'   : {
        fields:{
            wsid:     'VARCHAR(32) NOT NULL',
            module:   'VARCHAR(32) NOT NULL',
            params:   'TEXT'
        }, 
        primary_key:  'wsid,module'
    },
};