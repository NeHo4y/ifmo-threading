import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;

val file = sc.textFile("hdfs://namenode:9000/nasa/access_log_Jul95")

val conf = new Configuration()

conf.set("fs.defaultFS", "hdfs://namenode:9000")
val fs= FileSystem.get(conf)

val os = fs.create(new Path("/results/result3.txt"))

val myLogRegex = raw"""^(\S+) (\S+) (\S+) \[([\w/]+):([\w:]+\s[+\-]\d{4})\] "(.*)" (\d{3}) (\S+)""".r
val days = (8 to 31).toArray
val rdd = file.
	filter(line => line match {
		case myLogRegex(host, client_id, user_id, date, time, request, code, size) => (code(0) == '5' || code(0) == '4')
		case _ => false
	}).
	map(line => line match {
		// Only this case due to filter
		case myLogRegex(host, client_id, user_id, date, time, request, code, size) => (date, 1)
	}).
	reduceByKey((a, b) => a + b).
	// Cartesian with RDD [8, 9, ..., 31]
	cartesian(sc.parallelize(days)).
	// Filter ((date, n), day) to satisfy "Day of date is less than 7 days lower than 'day'"
	filter(a => a._1._1.split("/")(0).toInt <= a._2 && a._1._1.split("/")(0).toInt > (a._2 - 7)).
	// Turn ((date, n), day) to (day-7 - day, n)
	map(a => ((a._2 - 7) + "-" + (a._2), a._1._2)).
	reduceByKey((a, b) => a + b).
	sortBy(a => a._1.split("-")(0).toInt)
rdd.
	collect().
	foreach(pair => {val str = (pair._2 + ": " + pair._1 + "\n")
                         os.write(str.getBytes)
                        })
