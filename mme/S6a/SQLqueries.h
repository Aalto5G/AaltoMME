/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   SQLqueries.h
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  Strings with the SQL query templates
 *
 */


const char operators[]= "SELECT op,amf,name FROM operators WHERE (mcc, mnc) = (%u, %u)";

const char profileauth[] = "SELECT k,OPc FROM subscriber_profile WHERE (mcc, mnc, msin) = (%u, %u, x'%.10llu')";

const char authparams[] = "SELECT  subscriber_profile.k, subscriber_profile.opc, subscriber_profile.sqn, "
        "operators.op, operators.amf "
        "FROM subscriber_profile INNER JOIN operators "
        "ON subscriber_profile.mcc = operators.mcc AND subscriber_profile.mcc = %u "
        "AND subscriber_profile.mnc = operators.mnc and subscriber_profile.mnc = %u "
        "AND subscriber_profile.msin = x'%.10llu'";

const char insertAuthVector[] = "REPLACE INTO auth_vec values (%u, %u, %u, x'%.10llu',"
  "x'%s', x'%s', x'%s', x'%s', x'%s', x'%s', x'%s', x'%s'); "
  "UPDATE subscriber_profile SET sqn=x'%s', opc=x'%s' WHERE (mcc, mnc, msin) = (%u, %u, x'%.10llu')";

/*hex(ik) | hex(ck) | hex(rand) | hex(xres) | hex(autn) | hex(sqn) | hex(kasme) |hex(ak)*/

const char exists_auth_vec[]="SELECT EXISTS (SELECT '1' FROM auth_vec WHERE (mcc, mnc, msin) = (%u, %u, x'%.10llu') )";

const char get_auth_vec[]="SELECT rand, autn, xres, kasme FROM auth_vec WHERE (mcc, mnc, msin, ksi) = (%u, %u, x'%.10llu', %u)";

const char update_location[] = "UPDATE subscriber_profile "
        "SET mmec=CONV('%.2x',16,10), mmegi=CONV('%s',16,10), network_access_mode = '%u'"
        "WHERE (mcc, mnc, msin) = (%u, %u, x'%.10llu')";

const char get_subscriber_profile[] = "SELECT "
        "s.msisdn, s.ue_ambr_ul, s.ue_ambr_dl, s.apn_io_replacement, s.charging_characteristics, "
        "p.apn, p.pgw_allocation_type, p.vplmn_dynamic_address_allowed, p.eps_pdn_subscribed_charging_characteristics, "
        "p.pdn_addr_type, p.pdn_addr, p.subscribed_apn_ambr_dl, p.subscribed_apn_ambr_up, "
        "p.qci, p.qos_allocation_retention_priority_level,"
        "p.qos_allocation_retention_priority_preemption_capability, p.qos_allocation_retention_priority_preemption_vulnerability "
        "FROM subscriber_profile AS s INNER JOIN pdn_subscription_ctx AS p "
        "ON (s.mcc, s.mnc, s.msin) = (p.mcc, p.mnc, p.msin) "
        "WHERE (s.mcc, s.mnc, s.msin) = (%u, %u, x'%.10llu') and p.ctx_id = %u";
