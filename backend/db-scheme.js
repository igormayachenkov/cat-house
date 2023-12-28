exports.tables = {
    // WS CONFIG TABLES

    'hackers'   : {
        fields:{
            time:       'BIGINT NOT NULL',
            ip:         'VARCHAR(16) NOT NULL',
            body:       'VARCHAR(255) NOT NULL'
        }, 
    },

    'sensors'   : {
        fields:{
            time:       'BIGINT NOT NULL',
            ip:         'VARCHAR(16) NOT NULL',

            tempA:      'FLOAT NOT NULL',   // inside temperature 
            tempTarget: 'FLOAT NOT NULL',   // target temperature
            heating:    'TINYINT NOT NULL', // heating state
            tempB:      'FLOAT NOT NULL',   // outside temperature
            
            reason:     'TINYINT NOT NULL'  // Log reason (1:start, 2:timer, 3:change)
        }, 
        primary_key: 'time'
    },

};