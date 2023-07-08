

CONFIGURATION_NAME=$1
MY_PWD=$CONFIGURATION_NAME/$2
mkdir -p $CONFIGURATION_NAME/$2

cd $MY_PWD

mkdir -p TransMasterProblems
mkdir -p SubProblems

INSTANCE=../../../data/$2
LOG_FILE=$CONFIGURATION_NAME"_"$2.log

# echo $INSTANCE
../../../build/PHALS $INSTANCE | tee $LOG_FILE