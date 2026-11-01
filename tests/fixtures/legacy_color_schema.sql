CREATE TABLE projects (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, manufacturer TEXT NOT NULL DEFAULT '', location TEXT NOT NULL DEFAULT '', note TEXT NOT NULL DEFAULT '', status TEXT NOT NULL DEFAULT 'planned' CHECK(status IN ('planned','started','finished')), photo BLOB CHECK(photo IS NULL OR (typeof(photo)='blob' AND length(photo) BETWEEN 1 AND 10485760)));
PRAGMA application_id=1296191537;
PRAGMA user_version=1;
