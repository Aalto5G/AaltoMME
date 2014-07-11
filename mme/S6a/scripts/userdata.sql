-- MySQL dump 10.14  Distrib 10.0.3-MariaDB, for debian-linux-gnu (i686)
--
-- Host: localhost    Database: hss_lte_db
-- ------------------------------------------------------
-- Server version	10.0.3-MariaDB-1~wheezy-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `hss_lte_db`
--

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `hss_lte_db` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `hss_lte_db`;

--
-- Dumping data for table `auth_vec`
--

LOCK TABLES `auth_vec` WRITE;
/*!40000 ALTER TABLE `auth_vec` DISABLE KEYS */;
/*!40000 ALTER TABLE `auth_vec` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `operators`
--

LOCK TABLES `operators` WRITE;
/*!40000 ALTER TABLE `operators` DISABLE KEYS */;
INSERT INTO `operators` VALUES (244,7,'	','€€','Finish test operator');
/*!40000 ALTER TABLE `operators` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `pdn_subscription_ctx`
--

LOCK TABLES `pdn_subscription_ctx` WRITE;
/*!40000 ALTER TABLE `pdn_subscription_ctx` DISABLE KEYS */;
INSERT INTO `pdn_subscription_ctx` VALUES (244,7,'\0\0',0,'lemul.nsn.com','','','\0\0','','À¨8\0\0\0\0\0\0\0\0',100000,100000,1,1,'\0','\0');
/*!40000 ALTER TABLE `pdn_subscription_ctx` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Dumping data for table `subscriber_profile`
--

LOCK TABLES `subscriber_profile` WRITE;
/*!40000 ALTER TABLE `subscriber_profile` DISABLE KEYS */;
INSERT INTO `subscriber_profile` VALUES (244,7,'\0\0',358507777001,'\0\"3DUfwˆ™ª»ÌÝîÿ',NULL,'I7• \0','\0\0\0\0\0\0',NULL,NULL,NULL,100000,100000,NULL,NULL);
/*!40000 ALTER TABLE `subscriber_profile` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-07-12 19:37:26
