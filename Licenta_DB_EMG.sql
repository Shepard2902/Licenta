CREATE DATABASE EMG_DATA;
USE EMG_DATA;
CREATE TABLE EMGDATA (
    id INT AUTO_INCREMENT PRIMARY KEY,
    Muscle VARCHAR(100) NOT NULL,
    Exercise VARCHAR(100) NOT NULL,
    `Raw Value` DECIMAL(10, 4) NOT NULL,
    `Filtered Value` DECIMAL(10, 4) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
SHOW DATABASES;
SHOW TABLES;
DESCRIBE EMGDATA;
ALTER TABLE EMGDATA
RENAME COLUMN created_at TO added_at;
-- Alternative approach if you need to keep data
START TRANSACTION;

-- First add new columns
ALTER TABLE EMGDATA
ADD COLUMN `Raw No Contraction` DECIMAL(10, 4) NULL,
ADD COLUMN `Filtered No Contraction` DECIMAL(10, 4) NULL,
ADD COLUMN `Raw Contraction` DECIMAL(10, 4) NOT NULL DEFAULT 0,
ADD COLUMN `Filtered Contraction` DECIMAL(10, 4) NOT NULL DEFAULT 0,
ADD COLUMN `Weight` DECIMAL(6, 2) NULL;

-- Migrate data from old to new columns if needed
UPDATE EMGDATA SET
    `Raw Contraction` = `Raw Value`,
    `Filtered Contraction` = `Filtered Value`;

-- Then remove old columns
ALTER TABLE EMGDATA
DROP COLUMN `Raw Value`,
DROP COLUMN `Filtered Value`;

COMMIT;
ALTER TABLE EMGDATA
MODIFY COLUMN added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP AFTER Weight;
INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Pectoral',                -- Muscle name
    'Pushups',                  -- Exercise name
    2708,                  -- Raw value at rest
    5.83,                  -- Filtered value at rest
    2874,                  -- Raw value during contraction
    66.17,                  -- Filtered value during contraction
    80                     -- Weight in kg
);
INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Pectorals',                -- Muscle name
    'Dumbell Press',                  -- Exercise name
    2731,                  -- Raw value at rest
    0.39,                  -- Filtered value at restPRIMARYPRIMARY
    2983,                  -- Raw value during contraction
    142.16,                  -- Filtered value during contraction
    5.00                     -- Weight in kg
);

INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Biceps',                -- Muscle name
    'Curls',                  -- Exercise name
    2737,                  -- Raw value at rest
    18.92,                  -- Filtered value at rest
    3136,                  -- Raw value during contraction
    417.08,                  -- Filtered value during contraction
    5.00                     -- Weight in kg
);

INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Biceps',                -- Muscle name
    'Curls',                  -- Exercise name
    2739,                  -- Raw value at rest
    15.94,                  -- Filtered value at rest
    3291,                  -- Raw value during contraction
    843.1,                  -- Filtered value during contraction
    10.00                     -- Weight in kg
);

INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Lateral Deltoid',                -- Muscle name
    'Lateral Raises',                  -- Exercise name
    2732,                  -- Raw value at rest
    5.55,                  -- Filtered value at rest
    2974,                  -- Raw value during contraction
    268.78,                  -- Filtered value during contraction
    5.00                     -- Weight in kg
);

INSERT INTO EMGDATA (
    Muscle,
    Exercise,
    `Raw No Contraction`,
    `Filtered No Contraction`,
    `Raw Contraction`,
    `Filtered Contraction`,
    Weight
) VALUES (
    'Lateral Deltoid',                -- Muscle name
    'Lateral Raises',                  -- Exercise name
    2740,                  -- Raw value at rest
    44.92,                  -- Filtered value at rest
    3092,                  -- Raw value during contraction
    604.74,                  -- Filtered value during contraction
    10.00                     -- Weight in kg
);



SELECT * FROM EMGDATA;

