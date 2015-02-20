#!/bin/bash

executable_gecode="bin/Release/eps-gecode -mode stat -s "

NB_WORKERS=(10 20 30 40 80);
NB_PROBLEMS=(50 100 200);

DIRECTORY_INSTANCE="../minizinc_challenges"

DIRECTORY_RESULTS="instances_results_gecode"

mkdir -p ${DIRECTORY_RESULTS}

STRATEGY=(bab eps);
#STRATEGY=(bab restart);

while read F
do
	FILE=$(echo $F | cut -d":" -f1);
	
	echo Start the instance ${FILE}_bab"_1"
	${executable_gecode} -problems 1 -search bab -p 1 -o ${FILE}"_bab_1.o" ${DIRECTORY_INSTANCE}"/"${FILE}
	mv *.o ${DIRECTORY_RESULTS}/
	echo End for the instance ${FILE}_bab_1

	S="bab"
	for W in "${NB_WORKERS[@]}"
	do
		let problems=1
		
		echo Start the instance ${FILE}_$S_$W with $S		
		${executable_gecode} -problems ${problems} -search $S -p $W -o ${FILE}"_"$S"_"$W"_"${problems}".o" ${DIRECTORY_INSTANCE}"/"${FILE}
		mv *.o ${DIRECTORY_RESULTS}/
		echo End for the instance ${FILE}_$S_$W	
		
	done


	S="eps"
	for W in "${NB_WORKERS[@]}"
	do
		for P in "${NB_PROBLEMS[@]}"
		do
			let problems=$P*$W
			
			echo Start the instance ${FILE}_$S_$W with $S : ${problems} problems
			${executable_gecode} -problems ${problems} -search $S -p $W -o ${FILE}"_"$S"_"$W"_"${problems}".o" ${DIRECTORY_INSTANCE}"/"${FILE}
			mv *.o ${DIRECTORY_RESULTS}/
			echo End for the instance ${FILE}_$S_$W	
		done	
	done
	
done < "test.txt"
echo FINIIIIIIISH!!!
