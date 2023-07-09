
CONFIGURATION_NAME=$1
TIME_LIMIT=$3
MY_PWD=$CONFIGURATION_NAME"_"$TIME_LIMIT"/"$2

mkdir -p $MY_PWD
cd $MY_PWD

mkdir -p TransMasterProblems
mkdir -p SubProblems

INSTANCE=../../../data/$2
LOG_FILE=$CONFIGURATION_NAME"_"$TIME_LIMIT"_"$2.log

# echo $INSTANCE
../../../build/PHALS_$CONFIGURATION_NAME $INSTANCE | tee $LOG_FILE