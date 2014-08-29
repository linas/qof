--
-- Create the table below in a database named "my-qof-db" 
-- The C source code is expecting the database to have this name.
--
-- For example, in postgres, one woulddo the following:
--   Install the 'postgresql-client' package
--   Then run 'createdb my-qof-db'  to create the database,
--   and then, 'psql my-qof-db < my-instance.sql'
--


CREATE TABLE MyInstance (
	-- All Qof Entities use the GUID as the index.  The GUID field
	-- should be declared as either the primary key, or an index
	-- on the GUID field should be built.
	myGuid    CHAR(32) PRIMARY KEY,

	-- The QofMap code uses timestamps to determine whether the
	-- local or remote version of an object is newer when working
	-- with multi-user updates.  To get the time right, the version
	-- filed ***must*** be declared ***with time zone***.  Otherwise,
	-- users in different timezones will get conflicting, weird 
	-- behaviour.
	myVersion TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

	a         INT,
	b         SERIAL,
	memo      TEXT
);

INSERT INTO MyInstance (myGuid,a,b,memo) VALUES 
                ('fedcba0123456789abcdef0123456789', '0', '1', 'splatergy');

INSERT INTO MyInstance (myGuid,a,b,memo) VALUES 
                ('0123456789abcdef0123456789abcdef', '1', '2', 'blortarriffic');

INSERT INTO MyInstance (myGuid,a,b,memo) VALUES 
                ('abcdef0987654321abcdef0987654321', '2', '3', 'plosive');


