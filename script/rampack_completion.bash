# Bash completion script

_rampack_debug() {
	echo "prev: '$prev'" >debug.txt
	echo "cur: '$cur'" >>debug.txt
	echo "words: '${words[@]}'" >>debug.txt
	echo "cword: '$cword'" >>debug.txt
	echo "COMP_WORDBREAKS: '$COMP_WORDBREAKS'" >>debug.txt
	echo "COMP_WORDS: '$COMP_WORDS'" >>debug.txt
}

_rampack_verbosity() {
	COMPREPLY=($(compgen -W 'fatal error warn info verbose debug' -- ${cur}))
}

_rampack_snapshot_out() {
	COMPREPLY=($(compgen -W "\'ramsnap( \'wolfram( \'xyz(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_shape() {
	COMPREPLY=($(compgen -W "\'sphere( \'kmer( \'polysphere_banana( \'polysphere_lollipop( 
		\'polysphere_wedge( \'spherocylinder( \'polyspherocylinder_banana( \'smooth_wedge(
		\'polysphere( \'polyspherocylinder( \'generic_convex(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_shape_out() {
	COMPREPLY=($(compgen -W "\'wolfram( \'obj(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_observable() {
	COMPREPLY=($(compgen -W "number_density box_dimensions packing_fraction compressibility_factor
		energy_per_particle energy_fluctuations_per_particle nematic_order \'nematic_order(
		\'smectic_order( \'bond_order( rotation_matrix_drift temperature pressure
		\'fourier_tracker \'scoped(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_bulk_observable() {
	COMPREPLY=($(compgen -W "\'pair_density_correlation( \'pair_averaged_correlation(
		\'density_histogram(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_trajectory_out() {
	COMPREPLY=($(compgen -W "\'ramtrj( \'xyz(" -- "${cur/\'/\\\'}"))
	compopt -o nospace
}

_rampack_casino() {
	_split_longopt

	case "$prev" in
		-h | --help)
			# there should be no options together with --help, so cancel completion
			COMPREPLY=()
			return
			;;
		-i | --input)
			_filedir
			return
			;;
		-V | --verbosity)
			_rampack_verbosity
			return
			;;
		-s | --start-from)
			COMPREPLY=($(compgen -W '.first .last .auto' -- ${cur}))
			return
			;;
		-c | --continue)
			# value is optional; it is a number, so just pass
			;;
		-l | --log-file)
			_filedir
			return
			;;
		--log-file-verbosity)
			_rampack_verbosity
			return
			;;
	esac

	if [[ "$cur" == -* ]] || [[ "$cur" == "" ]]; then
		COMPREPLY=($(compgen -W '$( _parse_help "${rampack_cmd} casino" --help )' -- ${cur}))
	fi
}

_rampack_preview() {
	_split_longopt

	case "$prev" in
		-h | --help)
			# there should be no options together with --help, so cancel completion
			COMPREPLY=()
			return
			;;
		-i | --input)
			_filedir
			return
			;;
		-o | --output)
			_rampack_snapshot_out
			return
			;;
	esac

	if [[ "$cur" == -* ]] || [[ "$cur" == "" ]]; then
		COMPREPLY=($(compgen -W '$( _parse_help "${rampack_cmd} preview" --help )' -- ${cur}))
	fi
}

_rampack_shape_preview() {
	_split_longopt

	case "$prev" in
		-h | --help)
			# there should be no options together with --help, so cancel completion
			COMPREPLY=()
			return
			;;
		-i | --input)
			_filedir
			return
			;;
		-S | --shape)
			_rampack_shape
			return
			;;
		-l | --log-info)
			;;
		-o | --output)
			_rampack_shape_out
			;;
	esac

	if [[ "$cur" == -* ]] || [[ "$cur" == "" ]]; then
		COMPREPLY=($(compgen -W '$( _parse_help "${rampack_cmd} shape-preview" --help )' -- ${cur}))
	fi
}

_rampack_trajectory() {
	_split_longopt

	case "$prev" in
		-h | --help)
			# there should be no options together with --help, so cancel completion
			COMPREPLY=()
			return
			;;
		-i | --input)
			_filedir
			return
			;;
		-r | --run-name)
			COMPREPLY=($(compgen -W '.first .last' -- ${cur}))
			return
			;;
		-f | --auto-fix)
			# pass thorugh
			;;
		-V | --verbosity)
			_rampack_verbosity
			return
			;;
		-o | --output-obs)
			_filedir
			return
			;;
		-O | --observable)
			_rampack_observable
			return
			;;
		-b | --output-bulk-obs)
			_filedir
			return
			;;
		-B | --bulk-observable)
			_rampack_bulk_observable
			return
			;;
		-a | --averaging-start)
			COMPREPLY=()
			return
			;;
		-T | --max-threads)
			COMPREPLY=()
			return
			;;
		-s | --output-snapshot)
			_rampack_snapshot_out
			return
			;;
		-I | --log-info)
			# pass through
			;;
		-l | --log-file)
			_filedir
			return
			;;
		--log-file-verbosity)
			_rampack_verbosity
			return
			;;
		-t | --output-trajectory)
			_rampack_trajectory_out
			return
			;;
		-x | --truncate)
			COMPREPLY=()
			return
			;;
	esac

	if [[ "$cur" == -* ]] || [[ "$cur" == "" ]]; then
		COMPREPLY=($(compgen -W '$( _parse_help "${rampack_cmd} trajectory" --help )' -- ${cur}))
	fi
}

_rampack() {
	local cur prev words cword
	_get_comp_words_by_ref -n "=" cur prev words cword

	local rampack_cmd="$1"

	#_rampack_debug

	if [ ${#words[@]} -ge 3 ]; then
		cmd=${words[1]}

		case "$cmd" in
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

		return
	fi

	COMPREPLY=($(compgen -W "casino preview shape-preview trajectory -h --help" -- "$cur"))
}


complete -F _rampack rampack