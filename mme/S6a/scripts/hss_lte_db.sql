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
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Current Database: `hss_lte_db`
--

/*!40000 DROP DATABASE IF EXISTS `hss_lte_db`*/;

CREATE DATABASE /*!32312 IF NOT EXISTS*/ `hss_lte_db` /*!40100 DEFAULT CHARACTER SET latin1 */;

USE `hss_lte_db`;

--
-- Table structure for table `auth_vec`
--

DROP TABLE IF EXISTS `auth_vec`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `auth_vec` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `mcc` smallint(3) unsigned NOT NULL,
  `mnc` smallint(3) unsigned NOT NULL,
  `msin` binary(5) NOT NULL,
  `ik` binary(16) DEFAULT NULL,
  `ck` binary(16) DEFAULT NULL,
  `rand` binary(16) DEFAULT NULL,
  `xres` binary(8) DEFAULT NULL,
  `autn` binary(16) DEFAULT NULL,
  `sqn` binary(6) DEFAULT NULL,
  `kasme` binary(16) DEFAULT NULL,
  `ak` binary(6) DEFAULT NULL,
  PRIMARY KEY (`id`,`mcc`,`mnc`,`msin`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `operators`
--

DROP TABLE IF EXISTS `operators`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `operators` (
  `mcc` smallint(3) unsigned NOT NULL,
  `mnc` smallint(3) unsigned NOT NULL,
  `op` binary(16) DEFAULT NULL,
  `amf` binary(2) DEFAULT NULL,
  `name` varchar(20) DEFAULT NULL,
  PRIMARY KEY (`mcc`,`mnc`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `pdn_subscription_ctx`
--

DROP TABLE IF EXISTS `pdn_subscription_ctx`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `pdn_subscription_ctx` (
  `mcc` smallint(3) unsigned NOT NULL,
  `mnc` smallint(3) unsigned NOT NULL,
  `msin` binary(5) NOT NULL,
  `ctx_id` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `apn` varchar(30) DEFAULT NULL,
  `pgw_allocation_type` bit(1) DEFAULT NULL,
  `vplmn_dynamic_address_allowed` bit(1) DEFAULT NULL,
  `eps_pdn_subscribed_charging_characteristics` binary(2) DEFAULT NULL,
  `pdn_addr_type` bit(2) DEFAULT NULL,
  `pdn_addr` binary(12) DEFAULT NULL,
  `subscribed_apn_ambr_dl` int(10) unsigned DEFAULT NULL,
  `subscribed_apn_ambr_up` int(10) unsigned DEFAULT NULL,
  `qci` tinyint(3) unsigned DEFAULT NULL,
  `qos_allocation_retention_priority_level` tinyint(3) unsigned DEFAULT NULL,
  `qos_allocation_retention_priority_preemption_capability` bit(1) DEFAULT NULL,
  `qos_allocation_retention_priority_preemption_vulnerability` bit(1) DEFAULT NULL,
  PRIMARY KEY (`mcc`,`mnc`,`msin`,`ctx_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `subscriber_profile`
--

DROP TABLE IF EXISTS `subscriber_profile`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `subscriber_profile` (
  `mcc` smallint(3) unsigned NOT NULL,
  `mnc` smallint(3) unsigned NOT NULL,
  `msin` binary(5) NOT NULL,
  `msisdn` bigint(16) NOT NULL,
  `k` binary(16) DEFAULT NULL,
  `opc` binary(16) DEFAULT NULL,
  `sqn` binary(6) DEFAULT NULL,
  `imsisv` binary(8) DEFAULT NULL,
  `mmec` tinyint(3) unsigned DEFAULT NULL,
  `mmegi` smallint(5) unsigned DEFAULT NULL,
  `network_access_mode` tinyint(3) unsigned DEFAULT NULL,
  `ue_ambr_ul` int(10) unsigned DEFAULT NULL,
  `ue_ambr_dl` int(10) unsigned DEFAULT NULL,
  `apn_io_replacement` varchar(30) DEFAULT NULL,
  `charging_characteristics` binary(2) DEFAULT NULL,
  PRIMARY KEY (`mcc`,`mnc`,`msin`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-07-12 19:37:25
# DB access rights
grant delete,insert,select,update on hss_lte_db.* to hss@localhost identified by 'hss';
