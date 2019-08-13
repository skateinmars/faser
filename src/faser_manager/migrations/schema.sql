CREATE TABLE "schema_migration" (
"version" TEXT NOT NULL
);
CREATE UNIQUE INDEX "schema_migration_version_idx" ON "schema_migration" (version);
CREATE TABLE "players" (
"id" TEXT PRIMARY KEY,
"uid" TEXT NOT NULL,
"name" TEXT NOT NULL,
"active" bool NOT NULL,
"sensor_0" INTEGER NOT NULL,
"sensor_1" INTEGER NOT NULL,
"sensor_2" INTEGER NOT NULL,
"sensor_3" INTEGER NOT NULL,
"created_at" DATETIME NOT NULL,
"updated_at" DATETIME NOT NULL
);
CREATE TABLE "pads" (
"id" TEXT PRIMARY KEY,
"name" TEXT NOT NULL,
"port" TEXT NOT NULL,
"player_id" char(36),
"created_at" DATETIME NOT NULL,
"updated_at" DATETIME NOT NULL
);
