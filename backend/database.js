const mysql = require('mysql');

//------------------------------------------------------------------------------
// DATABASE wit connection pooling
// options:
//      host
//      user
//      password
//      database
class Database {
    constructor(database){
        this.options = {...gConfig.DATABASE, database}
        this.pool = mysql.createPool(this.options);
    }
    disconnect(){
        this.pool.end();
    }

    getDatabaseName(){
        return this.pool.config.connectionConfig.database;
    }

    // MySQL connection.query wrapped in Promise
    // return: results 
    async query(sql){
        const pool = this.pool;
        return new Promise(function(resolve,reject){
            pool.query(sql, function (error, results, fields) {
                if(error) reject(error);
                else resolve(results);
            });
        });
    }
    // return: results + fields
    async queryWithFields(sql){
        const pool = this.pool;
        return new Promise(function(resolve,reject){
            pool.query(sql, function (error, results, fields) {
                if(error) reject(error);
                else resolve({results:results, fields:fields});
            });
        });
    }
    
    // returns the transaction object
    async startTransaction(){
        const pool = this.pool;
        return new Promise(function(resolve,reject){
            pool.getConnection(function(err, connection) {
                if(err) reject(err);
                else connection.beginTransaction(function (error) {
                    if(error){
                        connection.release();
                        reject(error);
                    }
                    else resolve(new Transaction(connection));
                });
            });
        });
    }

    // Prepare INSERT SQL
    insertSQL(table,data){
        let keys = '';
        let vals = '';
        let comma= '';
        for(var key in data){
            keys += comma + key;
            vals += comma + JSON.stringify(data[key]);
            comma = ',';
        }
        return 'INSERT INTO '+table+' ('+keys+') VALUES ('+vals+')'
    }

    // Prepare UPDATE SQL
    updateSQL(table,data,where){
        let pairs = '';
        let comma= '';
        for(var key in data){
            pairs += comma + key+'='+JSON.stringify(data[key]);
            comma = ',';
        }
        return 'UPDATE '+table+' SET '+pairs+' WHERE '+where;
    }
}

//------------------------------------------------------------------------------
// TRANSACTION
// the connection must be:
//      - selected from the pool 
//      - withs started transaction
class Transaction {
    constructor(connection){
        this.connection = connection;
    }
    async query(sql){
        const connection = this.connection;
        return new Promise(function(resolve,reject){
            connection.query(sql, function (error, results, fields) {
                if(error) reject(error);
                else resolve(results);
            });
        });
    }
    async commit(){
        const connection = this.connection;
        return new Promise(function(resolve,reject){
            connection.commit(function (error) {
                connection.release();
                if(error) reject(error);
                else resolve();
            });
        });
    }
    async rollback(){
        const connection = this.connection;
        return new Promise(function(resolve,reject){
            connection.rollback(function () {
                connection.release();
                resolve();
            });
        });
    }
}


exports.Database = Database;
exports.Transaction = Transaction;
