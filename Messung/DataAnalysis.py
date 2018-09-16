'''
Titel: DataAnalysis.py
Autor: Vincent Piegsa
Datum: 16.09.2018
Beschreibung:
	- class Data: Daten einlesen und formatieren
	- class Database: Datenbank erstellen und mit Daten füllen
	- class DatabaseOperations: Min, Max etc. aus der Datenbank
	- class PlotData: Daten mit matplotlib als Grafik speichern
'''

import sqlite3
import os
import matplotlib.pyplot as plt


class Data:
	@staticmethod
	def readData(filename):
		file = open(filename, 'r')
		data = file.read()
		file.close()

		return data

	@staticmethod
	def splitData(data, index):
		data = data.split('\n')

		content = []
		counter = 0

		for element in data:
			if counter % 2 == 0:
				content.append(element.split(';')[index])

			counter += 1

		return content


class Database:
	@staticmethod
	def create_db():
		cursor.execute("CREATE TABLE IF NOT EXISTS sensorValues(date STRING, time STRING, temperature FLOAT, humidity FLOAT);")

	@staticmethod
	def insert_db(date, time, temperature, humidity):
		cursor.execute("INSERT INTO sensorValues(date, time, temperature, humidity) VALUES(?, ?, ?, ?);",
			(date, time, temperature, humidity))
		connection.commit()

	@staticmethod
	def populate_db():
			data = readData('DATALOG.txt')

			if not os.path.isfile("data.db"):
				print("creating database")
				
				connection = sqlite3.connect('data.db')
				cursor = connection.cursor()

				create_db()

			else:
				connection = sqlite3.connect('data.db')
				cursor = connection.cursor()
			
			date = Data.splitData(data, 0)
			time = Data.splitData(data, 1)
			temperature = Data.splitData(data, 2)
			humidity = Data.splitData(data, 3)

			for index in range(len(date) - 1):
				insert_db(date[index], time[index], temperature[index], humidity[index])
				
			cursor.close()
			connection.close()
	

class DatabaseOperations:
	@staticmethod
	def fetch():
		connection = sqlite3.connect('data.db')
		cursor = connection.cursor()

		cursor.execute("SELECT * FROM sensorValues;")
		rows = cursor.fetchall()

		for row in rows:
			print(row)

		cursor.close()
		connection.close()

	@staticmethod	
	def avg(dates):
		data = []

		connection = sqlite3.connect('data.db')
		cursor = connection.cursor()

		for date in dates:
			cursor.execute("SELECT AVG(temperature), AVG(humidity) FROM sensorValues WHERE date = '{}';".format(date))
			rows = cursor.fetchall()

			for row in rows:
				data.append([date, row])

		cursor.close()
		connection.close()

		return data

	@staticmethod
	def max(argument):
		connection = sqlite3.connect('data.db')
		cursor = connection.cursor()

		cursor.execute("SELECT date, time, MAX({}), humidity FROM sensorValues;".format(argument))

		rows = cursor.fetchall()

		for row in rows:
			print(row)

		cursor.close()
		connection.close()

	@staticmethod
	def min(argument):
		connection = sqlite3.connect('data.db')
		cursor = connection.cursor()

		cursor.execute("SELECT date, time, min({}), humidity FROM sensorValues;".format(argument))

		rows = cursor.fetchall()

		for row in rows:
			print(row)


		cursor.close()
		connection.close()

	@staticmethod
	def dayAverage(date, argument):
		data = []

		connection = sqlite3.connect('data.db')
		cursor = connection.cursor()

		for time in range(0, 24):
			time = str(time) + ':'
			cursor.execute("SELECT AVG({}) FROM sensorValues WHERE date = '{}' AND time LIKE '{}%';".format(argument, date, time))

			data.append((time, cursor.fetchall()[0][0]))

		cursor.close()
		connection.close()

		return data

class PlotData:
	@staticmethod
	def averageWeek():
		dates = ['6.9.2018', '7.9.2018', '8.9.2018', '9.9.2018', '10.9.2018', '11.9.2018', '12.9.2018', '13.9.2018']

		data_avg = DatabaseOperations.avg(dates)

		x_label = []
		x1 = [0, 3, 6, 9, 12, 15, 18, 21]
		x2 = [1, 4, 7, 10, 13, 16, 19, 22]
		tmp_bar = []
		hdt_bar = []

		for element in data_avg:
			date = element[0][:-5]
			tmp = element[1][0]
			hdt = element[1][1]

			x_label.append(date)
			tmp_bar.append(tmp)
			hdt_bar.append(hdt)


		#plt.bar(x1, tmp_bar, label='Temperatur [°C]', color='r')
		plt.bar(x2, hdt_bar, label='Luftfeuchtigkeit [%]', color='b')

		plt.title('Wochendurchschnitt')
		plt.xlabel('Datum')
		plt.ylabel('Luftfeuchtigkeit [%]')
		plt.legend(loc = 'lower left')

		plt.xticks(x1, x_label)

		plt.savefig('humidityAVG.pdf', format='pdf')

	def averageDay():
		dates = ['6.9.2018', '7.9.2018', '8.9.2018', '9.9.2018', '10.9.2018', '11.9.2018', '12.9.2018', '13.9.2018']
	
		for date in dates:
			data = DatabaseOperations.dayAverage(date, 'humidity')

			x = [i for i in range(24)]
			x_label = []
			y = []

			for item in data:
				x_label.append(item[0] + '00')

				if item[1] != None:
					y.append(item[1])
				else:
					y.append(0)

			plt.bar(x, y, label='Luftfeuchtigkeit', color='b')

			plt.title('Tagesdurchschnitt: {}'.format(date))

			plt.xlabel('Uhrzeit')
			plt.ylabel('Luftfeuchtigkeit [%]')
			plt.ylim(50, 70)

			plt.savefig('hdt-{}.pdf'.format(date), format='pdf')


if __name__ == '__main__':
	dates = ['6.9.2018', '7.9.2018', '8.9.2018', '9.9.2018', '10.9.2018', '11.9.2018', '12.9.2018', '13.9.2018']
	
	# -Hier beliebige Funktion aufrufen- # 
