#compdef _rampack rampack


_rampack_verbosity=(fatal error warn info verbose debug)

_rampack_snapshot_output=(ramsnap wolfram xyz)

_rampack_shape=(sphere kmer polysphere_banana polysphere_lollipop polysphere_wedge spherocylinder
	polyspherocylinder_banana smooth_wedge polysphere polyspherocylinder generic_convex polyhedral_wedge)

_rampack_shape_output=(wolfram obj)

_rampack_observable=(number_density box_dimensions packing_fraction compressibility_factor energy_per_particle
	energy_fluctuations_per_particle nematic_order smectic_order bond_order rotation_matrix_drift temperature
	pressure fourier_tracker)

_rampack_bulk_observable=(pair_density_correlation pair_averaged_correlation density_histogram probability_evolution)

_rampack_trajectory_output=(ramtrj xyz)

function _rampack_run_names {
	local -a run_names=(".first" ".last")

	if [[ "$1" == "--with-auto" ]]; then
		run_names+=(".auto")
	fi

	for ((i = 1; i <= $#words; i++))
	do
		if [[ "${words[i]}" == -i* || "${words[i]}" == --input || "${words[i]}" == --input=* ]]; then
			local input_file
			if [[ "${words[i]}" == "-i" || "${words[i]}" == "--input" ]]; then
				input_file="${words[i+1]}"
			elif [[ "${words[i]}" == -i* ]]; then
				input_file="${words[i]:2}"
			else
				input_file="${words[i]:8}"
			fi

			local -a input_run_names
			input_run_names=($(rampack preview -i "$input_file" -r -V warn))
			[[ $? == 0 ]] && run_names+=("${input_run_names[@]}")
		fi
	done

	_describe "run names" run_names
}

function _rampack_casino {
	_arguments \
		'(- *)'{-h,--help}'[print help]' \
		'(-i --input)'{-i,--input=}'[input file]: :_files' \
		'(-V --verbosity)'{-V,--verbosity=}'[logging verbosity]: :'"(${_rampack_verbosity[*]})" \
		'(-s --start-from)'{-s,--start-from=}'[starting run]: :{_rampack_run_names --with-auto}' \
		'(-c --continue)'{-c,--continue=}'[continue for a number of cycles \[0: unchanged (implicit value)\]]:: : ' \
		'(-l --log-file)'{-l,--log-file=}'[file to log to (apart from stdout)]: :_files' \
		'--log-file-verbosity=[verbosity of the log file (default: info)]: :'"(${_rampack_verbosity[*]})"
}

function _rampack_preview {
	_arguments \
		'(- *)'{-h,--help}'[print help]' \
		'(-i --input)'{-i,--input=}'[input file]: :_files' \
		'(-V --verbosity)'{-V,--verbosity=}'[logging verbosity]: :'"(${_rampack_verbosity[*]})" \
		'*'{-o,--output=}'[snapshot output spec]: :'"(${_rampack_snapshot_output[*]})" \
		'(-r --run-names)'{-r,--run-names}'[output run names from the input file to stdout]'
}

function _rampack_shape_preview {
	_arguments \
		'(- *)'{-h,--help}'[print help]' \
		'(-i --input)'{-i,--input=}'[input file]: :_files' \
		'(-l --log-info)'{-l,--log-info}'[print information about the shape on stdout]' \
		'(-S --shape)'{-S,--shape=}'[manually specified shape]: :'"(${_rampack_shape[*]})" \
		'*'{-o,--output=}'[shape model output spec]: :'"(${_rampack_shape_output[*]})"
}


function _rampack_trajectory {
	_arguments \
		'(- *)'{-h,--help}'[print help]' \
		'(-i --input)'{-i,--input=}'[input file]: :_files' \
		'(-r --run-name)'{-r,--run-name=}'[name of the run to analyze trajectory]: :_rampack_run_names' \
		'(-f --auto-fix)'{-f,--auto-fix}'[if specified, trajectory fix will be attempted]' \
		'(-V --verbosity)'{-V,--verbosity=}'[logging verbosity]: :'"(${_rampack_verbosity[*]})" \
		'(-o --output-obs)'{-o,--output-obs=}'[observables output file name]: :_files' \
		'*'{-O,--observable=}'[observable(s) to calculate]: :'"(${_rampack_observable[*]})" \
		'(-b --output-bulk-obs)'{-b,--output-bulk-obs=}'[bulk observables output file name pattern]: :_files' \
		'*'{-B,--bulk-observable=}'[bulk observable(s) to calculate]: :'"(${_rampack_bulk_observable[*]})" \
		'(-a --averaging-start)'{-a,--averaging-start=}'[cycle number to start averaging]: : ' \
		'(-T --max-threads)'{-T,--max-threads=}'[number of threads to use (0 - use max) (default: 1)]: : ' \
		'*'{-s,--output-snapshot=}'[last snapshot output spec]: :'"(${_rampack_snapshot_output[*]})" \
		'(-I --log-info)'{-I,--log-info}'[print trajectory info]' \
		'(-l --log-file)'{-l,--log-file=}'[log additionally to a given file]: :_files' \
		'--log-file-verbosity=[log file verbosity]: :'"(${_rampack_verbosity[*]})" \
		'*'{-t,--output-trajectory=}'[trajectory output spec]: :'"(${_rampack_trajectory_output[*]})" \
		'(-x --truncate)'{-x,--truncate=}'[truncate trajectory on a given cycle number]: : '
}

function _rampack {
	local line
	local -a subcommands=(
		"casino\:Monte\ Carlo\ integration"
		"preview\:preview\ of\ the\ initial\ configuration"
		"shape-preview\:preview\ of\ the\ shape"
		"trajectory\:visualization\ and\ analysis\ of\ recorded\ trajectories"
		{-h,--help,help}"\:general\ help"
		{-v,--version,version}"\:shows\ current\ version"
	)

	_arguments \
		"1:subcommand:((${subcommands[*]}))" \
		"*:: :->args"

	case ${line[1]} in
		casino)
			_rampack_casino
			;;
		preview)
			_rampack_preview
			;;
		shape-preview)
			_rampack_shape_preview
			;;
		trajectory)
			_rampack_trajectory
			;;
	esac
}