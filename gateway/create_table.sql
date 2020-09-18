CREATE TABLE StoredDevices (
	id int PRIMARY KEY,
	address  VARCHAR(25) NOT NULL,
    name VARCHAR(20),
    device_type VARCHAR(20)
);

INSERT INTO StoredDevices VALUES (0, " -1 null drive", "John Doe", "microwave");


INSERT INTO StoredDevices VALUES (1, "100 ainslie drive", "Justin Whittam-Geskes", "Smoke Alarm");