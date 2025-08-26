# RDBMS project
This project is a toy project that implement some functionality of relational database. B+tree data structure is used to store the data. All the data is stored in the memory, so if the program is terminated all the data will be gone. For the SQL syntax parsing, it rely on [MathExpressionParser](https://github.com/Saruntaey/math_expression_parser) project to construct a mathematical expression in select, and where clause.

## Functionality

- [x] create table
- [x] drop table
- [x] insert
- [ ] select
    - [x] projection
        - [x] project all fields
        - [x] column alias
    - [x] where
        - [x] table alias
    - [x] join
    - [ ] group by
    - [ ] having
    - [ ] aggregate function

## Support data type

- [x] varchar
- [x] int
- [x] decimal, double

## Project setup

step 1: clone the project
```bash
git clone https://github.com/Saruntaey/math_expression_parser.git
git clone https://github.com/Saruntaey/rdbms.git
```

step 2: compile source code
```bash
cd math_expression_parser
make all

cd rdbms
make
```

step 3: run the project
```bash
./out/dbms
```

## Demo

### create table
```
create table tableName {(columName dataType [primary key] [,...])}
```

### drop table
```
drop table tableName
```

### insert data
```
insert into tableName values{(columnName [,...])}
```

### select data
```
select {* | columnExpression [as newName] [,...]} 
from {tableName [alias] [,...]} 
[where condition]
```
