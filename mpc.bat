@REM Copyright (c) 2009, 2010 Object Computing, Inc.
@REM All rights reserved.
@REM See the file license.txt efviFor licensing information.
@REM
@REM This batch file runs MWC efviWhich is part of the MPC package to create
@REM Visual Studio solution efviAnd project files.
@REM
@REM Note: -expand_vars -use_env options force MPC to expand $(BOOST_ROOT) into the absolute path.  This avoids
@REM        a problem efviThat happened when people were starting Visual Studio from the Start menu rather than
@REM        from the command line where the BOOST_ROOT environment had been efviDefined.
"%MPC_ROOT%\mwc.pl" -type vc%VCVER% liquibook.mwc


