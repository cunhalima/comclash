CREATE TABLE tab_segment(
  segid       INTEGER PRIMARY KEY NOT NULL,
  name        TEXT NOT NULL UNIQUE,
  class       INTEGER NOT NULL,
  align       INTEGER NOT NULL,
  use         INTEGER NOT NULL,
  address     INTEGER NOT NULL,
  priority    INTEGER NOT NULL,
  UNIQUE(address, priority)
);

CREATE TABLE tab_module(
  modid       INTEGER PRIMARY KEY NOT NULL,
  name        TEXT NOT NULL UNIQUE
);

CREATE TABLE tab_modslice(
  address     INTEGER PRIMARY KEY NOT NULL,
  size        INTEGER NOT NULL,
  module      INTEGER NOT NULL,
  segment     INTEGER NOT NULL,
  FOREIGN KEY(module) REFERENCES tab_module(modid) ON DELETE RESTRICT,
  FOREIGN KEY(segment) REFERENCES tab_segment(segid) ON DELETE RESTRICT
);

CREATE TABLE tab_label(
  labid       INTEGER PRIMARY KEY NOT NULL,
  address     INTEGER UNIQUE,
  name        TEXT UNIQUE
);

CREATE TABLE tab_reloc(
  address     INTEGER PRIMARY KEY NOT NULL,
  size        INTEGER NOT NULL,
  label       INTEGER NOT NULL,
  disp        INTEGER NOT NULL,
  FOREIGN KEY(label) REFERENCES tab_label(labid) ON DELETE RESTRICT
);
