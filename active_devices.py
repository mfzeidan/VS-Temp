import MySQLdb
import time
import datetime
from pytz import timezone

##the_time_frame is in minutes

def old_devices(the_time_frame):

	timeframe = the_time_frame

	con = MySQLdb.connect(host='159.203.165.37', port=3306,
		user='root',
		passwd='dataworks',
		db='login')
	cur = con.cursor()

	#print datetime.datetime.now().strftime("%s")

	x = datetime.datetime.now() - datetime.timedelta(minutes=timeframe)

	print str(timeframe) + " minutes ago"

	old_time = x.strftime("%s")
	print old_time
	#old_time = 1483900959

	sql_statement = """select distinct deviceID from device_msg_loggerv2 where ts < %s """ % (old_time)
	#sql_statement="""select email, deviceID from email_logging_qa"""
	#sql_statement = """select email from email_logging"""
	print sql_statement


	cur.execute(sql_statement)

	rows = cur.fetchall()
	print len(rows)

	for r in rows:
		print "an inactive device has been found"
		deviceID = r[0]
		username_sql = """select username from device_mgmt where deviceID = %s""" % (deviceID)
		#print username_sql
		cur.execute(username_sql)
		username_result = cur.fetchall()
		for x in username_result:
			if len(x) != 0:
				username = x[0]
				print username
				email_sql = """select email from user where username = '%s'""" % (username)
				print email_sql
				cur.execute(email_sql)
				email_result = cur.fetchall()
				for email in email_result:
					user_email = email[0]
					ts = int(time.time())
					print ts
					#####
					check_sql = """select email from device_status where email = '%s' and deviceID = %s""" % (user_email,deviceID)
					print check_sql
					cur.execute(check_sql)
					check_result = cur.fetchall()
					for checks in check_result:
						print checks
						
					if len(check_result) == 0:
						sql_logging = """insert into device_status (email, username, deviceID, ts) values ('%s','%s',%s,%s)""" % (user_email, username,deviceID,ts)
						cur.execute(sql_logging)
						con.commit()
					else:
						print "this record already exists"





				

#log email, username,deviceID and current ts into a table after an email has been sent to the email
#evey hour check to see if a new message has come in


#send a welcome back email once they turn their device back on :)



def mail_to_send(to,deviceID):	
	import smtplib
	from email.MIMEMultipart import MIMEMultipart
	from email.MIMEText import MIMEText
	 
	 
	fromaddr = "smokertempmail@gmail.com"
	
	#to = "mfzeidan@gmail.com"

	toaddr = to
	msg = MIMEMultipart()
	msg['From'] = fromaddr
	msg['To'] = to
	msg['Subject'] = "VoltSync Sensor Alert!"
	 
	body = "Your device " + str(deviceID) +  " has been turned back on. Thanks very much!"
	msg.attach(MIMEText(body, 'plain'))
	 
	server = smtplib.SMTP('smtp.gmail.com:587')
	server.ehlo()
	server.starttls()
	server.login(fromaddr, "44sdsdsdSD")
	text = msg.as_string()
	server.sendmail(fromaddr, toaddr, text)
	server.quit()
	












#this is the total devices that are currently "offline"



#for each device see if there has been an update recently






def clear_restarted_devices(time_spread):



	con = MySQLdb.connect(host='159.203.165.37', port=3306,
		user='root',
		passwd='dataworks',
		db='login')
	cur = con.cursor()




	device_status_sql = """select deviceID from device_status"""

	cur.execute(device_status_sql)

	device_results = cur.fetchall()

	print "go"


	device_status_sql = """select deviceID from device_status"""

	cur.execute(device_status_sql)


	for devices in device_results:
		device_var = devices[0]

		device_sql = """select ts from device_msg_loggerv3 where deviceid = %s order by datetime desc limit 1""" % (device_var)

		cur.execute(device_sql)
		
		device_queried = cur.fetchall()
		print "running"

		for dev in device_queried:
			loop_dev = devices[0]
			loop_time =  dev[0]
			ts = int(time.time())
			print devices[0]	

			dev_diff = ts - loop_time
			print dev_diff

			if dev_diff > time_spread:

				con.commit()

				

				email_lookup = """select email from device_status where deviceId = %s""" % (loop_dev)
				cur.execute(email_lookup)
				restarted_devices = cur.fetchall()
				for restart_dev in restarted_devices:
					print "an email will be sent"
					email_restart = restart_dev[0]
					print email_restart
					print loop_dev
					print "email sent"

					mail_to_send(email_restart, loop_dev)
	
					delete_device_inactive = """delete from device_status where deviceid = %s""" % (loop_dev)

					cur.execute(delete_device_inactive)

old_devices(10)

time.sleep(3)


clear_restarted_devices(10)


