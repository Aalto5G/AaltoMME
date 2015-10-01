#######################################
# The list of executables we are building seperated by spaces
# the 'bin_' indicates that these build products will be installed
# in the $(bindir) directory. For example /usr/bin
#bin_PROGRAMS=exampleProgram
bin_PROGRAMS=mme

#######################################
# Build information for each executable. The variable name is derived
# by use the name of the executable with each non alpha-numeric character is
# replaced by '_'. So a.out becomes a_out and the appropriate suffex added.
# '_SOURCES' for example.

# Sources
#AM_CXXFLAGS = $(INTI_CFLAGS)
#$(GLIB_CFLAGS) $(GOBJECT_CFLAGS)
mme_SOURCES = Subscription.c \
			  S1/MME_S1.c \
			  S1/S1Assoc.c \
			  S1/S1Assoc_FSMConfig.c \
			  S1/S1Assoc_Active.c \
			  S1/S1Assoc_NotConfigured.c \
			  S1/ECMSession.c \
			  S1/ECMSession_FSMConfig.c \
			  S1/ECMSession_Idle.c \
			  S1/ECMSession_Connected.c \
			  S1/NAS/EMMCtx.c \
			  S1/NAS/EMM_FSMConfig.c \
			  S1/NAS/EMM_Deregistered.c \
			  S1/NAS/EMM_DeregisteredInitiated.c \
			  S1/NAS/EMM_CommonProcedureInitiated.c \
			  S1/NAS/EMM_SpecificProcedureInitiated.c \
			  S1/NAS/EMM_Registered.c \
			  S1/NAS/EPS_Session.c \
			  S1/NAS/ESM_Active.c \
			  S1/NAS/ESM_ActivePending.c \
			  S1/NAS/ESM_BearerContext.c \
			  S1/NAS/ESM_FSMConfig.c \
			  S1/NAS/ESM_Inactive.c \
			  S1/NAS/ESM_InactivePending.c \
			  S1/NAS/ESM_ModifyPending.c \
			  S1/NAS/NAS_EMM.c \
			  S1/NAS/NAS_ESM.c \
			  S6a/hmac/sha2.c \
			  S6a/hmac/hmac_sha2.c \
			  S6a/milenage/aes.c \
			  S6a/milenage/milenage.c \
			  S6a/MME_S6a.c \
			  S6a/HSS.c \
			  S11/MME_S11.c \
			  S11/S11_User.c \
			  S11/S11_FSMConfig.c \
			  S11/S11_NoCtx.c \
			  S11/S11_WCtxRsp.c \
			  S11/S11_UlCtx.c \
			  S11/S11_WModBearerRsp.c \
			  S11/S11_Ctx.c \
			  S11/S11_WDel.c \
			  MME_test.c commands.c \
			  ../Common/logmgr.c \
			  MME.c \
			  MMEutils.c \
			  nodemgr.c \
			  Controller/MME_Controller.c

# Linker options
mme_LDFLAGS = $(top_srcdir)/libgtp/libgtp.la  $(top_srcdir)/S1AP/libs1ap.la $(top_srcdir)/NAS/src/libnas.la `mysql_config --libs_r`

mme_LDADD = -levent -lgtp -ls1ap -lnas -lconfig -lsctp $(GLIB_LIBS) $(GOBJECT_LIBS) #$(INTI_LIBS)

# Compiler options
mme_CFLAGS = `mysql_config --cflags` \
			 $(GLIB_CFLAGS) $(GOBJECT_CFLAGS) \
			 -I$(top_srcdir)/libgtp/include \
			 -I$(top_srcdir)/mme \
			 -I$(top_srcdir)/Common \
			 -I$(top_srcdir)/mme/S1 \
			 -I$(top_srcdir)/mme/S6a/ \
			 -I$(top_srcdir)/mme/S6a/hmac \
			 -I$(top_srcdir)/mme/S6a/milenage \
			 -I$(top_srcdir)/S1AP/shared \
			 -I$(top_srcdir)/mme/S1/NAS \
			 -I$(top_srcdir)/NAS/shared \
			 -I$(top_srcdir)/mme/S11 \
			 -I$(top_srcdir)/mme/Controller
