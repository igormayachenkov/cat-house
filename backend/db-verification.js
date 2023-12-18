const { Database } = require("./database.js");
const { Table } = require('console-table-printer');

//-----------------------------------------------------------------------------------
// VERIFY / CREATE DATABASE
// global.db variable must be set
// ex: global.db = new Database("mydatabase")

exports.verifyDatabase = async function(tables){
    // Verify db existance (or create database)
    await verifyDatabaseExistance();

    // Read db scheme
    let actualTables = await queryActualTables();
    // Create non-existed tables
    let changed = await createNonExistedTables(tables,actualTables);
    if(changed)
        actualTables = await queryActualTables(); // reread structure
    
    // Check table schemes
    await checkTableSchemes(tables)
}

const verifyDatabaseExistance = async function(){
    try{
        await db.query('SHOW TABLES');
    }catch(err){
        if(err.code=='ER_BAD_DB_ERROR'){
            //throw 'Create the database "'+db.getDatabaseName()+'" first';
            let dbnew = new Database()
            let sql = 'CREATE DATABASE '+db.getDatabaseName();
            await dbnew.query(sql);
            dbnew.disconnect();    
            console.log('*** mysql> '+sql);    
        }else 
            throw err;
    }
}

const queryActualTables = async function(){
    let results = await db.query('SHOW TABLE STATUS');
    let tables = {};

    results.forEach(row => {
        tables[row.Name] = {
            rows            : row.Rows, // always 0 for innodb
            data_length     : row.Data_length,
            avg_row_length  : row.Avg_row_length
        };
    });
    return tables;
}

const createNonExistedTables = async function(tables,actualTables){
    var changed = false;
    // create not existed tables
    for(let name in tables){
        if(!actualTables[name]){
            changed = true;
            let table = tables[name];
            // Create table
            let sql = createTableSQL(name, table);
            console.log('*** mysql> '+sql);
            await db.query(sql);
            // Rin init scripts for the table
            if(table.init){
                for(var i=0; i<table.init.length; i++){
                    await db.query(table.init[i]);
                }
            }
        }
    }
    return changed;
}
//-------------------------------------------------------------------------
// COMMON TABLE TRUCTURE
// Fill all property list [{field,type}] for table creation/verification
const getAllFields = function(table){
    let fields = []

    // Custom fields from table scheme
    for(const name in table.fields)
        fields.push({field: name, type: table.fields[name]});

    return fields;
}

//-------------------------------------------------------------------------
// Fill CREATE TABLE SQL by table scheme
const createTableSQL = function(name, table){
    let sql = 'CREATE TABLE '+name+' ('
    
    // Fields
    let fields = getAllFields(table)
    let comma = ''
    for(const name in fields){
        sql += comma + printFieldSQL(fields[name]);
        comma = ', '
    }

    // Primary key
    if(table.primary_key){
        // use defined primary key
        sql += ', PRIMARY KEY ('+table.primary_key+')';
    }else if(typeof table.chrono !== 'undefined'){
        // use common primary key for prop-table
        sql += ', PRIMARY KEY (id'+(table.chrono?',time':'')+')';
    }

    sql += ')';
    return sql;
}

const printFieldSQL = function(fld){
    let str = fld.field + ' ' +fld.type
    str = str.replace('BIGINT(20)','BIGINT')
    str = str.replace('INT(11)',   'INT')
    str = str.replace('TINYINT(4)','TINYINT')
    return str
}

const checkTableSchemes = async function(tables){
    let error = false
    for(let name in tables){
        let diffs = await verifyTableScheme(name, tables[name]);
        if(diffs){
            error = true
            console.log('----- '+name+' ----- WRONG TABLE SCHEME:')
            let table = new Table({ columns: [
                { name: 'required',	alignment: 'left'}, 
                { name: 'actual',   alignment: 'left' }
            ]});
            table.addRows(diffs);
            table.printTable();
        }
    }
    if(error) throw "Wrong database scheme"
}

const verifyTableScheme = async function(name, prop){
    // Get actual fields
    let results = await db.query('SHOW COLUMNS FROM '+name);
    let fieldsActual = []
    results.forEach(row=>{
        fieldsActual.push({
            field: row.Field, 
            type : row.Type.toUpperCase() + (row.Null=='NO'?' NOT NULL':'')});
            //type : row.Type.toUpperCase() + (row.Null=='NO'?' NOT NULL':'') + (row.Extra=='auto_increment'?' AUTO_INCREMENT':'')});
    });

    // Get required fields
    let fieldsRequired = getAllFields(prop)

    // Compare two arrays
    let equal=true;
    let diffs=[];

    for(var i=0; i<fieldsRequired.length||i<fieldsActual.length; i++){
        const fldR = fieldsRequired[i];
        const fldA = fieldsActual[i];
        const sqlR = fldR?printFieldSQL(fldR):undefined
        const sqlA = fldA?printFieldSQL(fldA):undefined
        if(sqlR!=sqlA) equal=false;

        diffs.push({
            required: sqlR, 
            actual  : sqlA
        });
    
    }
    if(equal) return null;
    else      return diffs;
}