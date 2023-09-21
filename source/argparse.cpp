#include "asx/argparse.hpp"
#include "asx/assert.hpp"

#include <jclib/algorithm.h>

#include <sstream>
#include <filesystem>

namespace asx
{
	ArgumentParser::ParsedArgument ArgumentParser::ParseResult::get(const std::string& _label) const
	{
		auto& _parsedValuesStorage = this->parsed_values_;
		auto& _parsedArgumentStorage = this->parsed_arguments_;
		auto& _labelledArgumentPositionStorage = this->labelled_argument_positions_;

		// Find argument position using its label
		const auto _parsedArgumentPosition = _labelledArgumentPositionStorage.at(_label);

		// Grab argument info 
		const auto& _parsedArgumentInfo = _parsedArgumentStorage.at(_parsedArgumentPosition);

		// Construct span to view values
		const auto _parsedArgumentValues = ParsedArgument::view_type
		(
			_parsedValuesStorage.begin() + _parsedArgumentInfo.offset,
			_parsedValuesStorage.begin() + _parsedArgumentInfo.offset + _parsedArgumentInfo.count
		);

		// Construct parsed argument object
		return ParsedArgument(_parsedArgumentValues);
	};

	void ArgumentParser::ParseResult::set_positional_argument(std::span<const ParsedValue> _values, std::string _label)
	{
		auto& _parsedValuesStorage = this->parsed_values_;
		auto& _parsedArgumentStorage = this->parsed_arguments_;
		auto& _labelledArgumentPositionStorage = this->labelled_argument_positions_;

		auto& _numPositionalArgs = this->num_positional_args_;


		// Check that we haven't set any strictly named argument values yet
		ASX_CHECK(_numPositionalArgs == _parsedArgumentStorage.size());

		// Determine parsed value offset/count into storage
		const auto _valueOffset = _parsedValuesStorage.size();
		const size_t _valueCount = _values.size();

		// Construct parsed argument info
		auto _parsedArgument = ParsedArgumentInfo
		{
			.offset = _valueOffset,
			.count = _valueCount
		};

		// Add values to parsed value storage.
		_parsedValuesStorage.insert(_parsedValuesStorage.end(), _values.begin(), _values.end());

		// Where the parsed argument info is being stored.
		const auto _parsedArgumentStorageIndex = _parsedArgumentStorage.size();

		// Add info to parsed argument storage
		_parsedArgumentStorage.push_back(_parsedArgument);

		// If a label was specified we can add it to the labelled arguments
		if (!_label.empty())
		{
			// Add to labelled storage
			const auto [it, _noCollision] = _labelledArgumentPositionStorage.insert_or_assign
			(
				std::move(_label), _parsedArgumentStorageIndex
			);

			// Check for no name collision
			ASX_CHECK(_noCollision);
		};

		// Indicate an additional positional arg was parsed
		++_numPositionalArgs;
	};

	void ArgumentParser::ParseResult::set_named_argument(std::span<const ParsedValue> _values, std::string _label)
	{
		ASX_CHECK(!_label.empty());

		auto& _parsedValuesStorage = this->parsed_values_;
		auto& _parsedArgumentStorage = this->parsed_arguments_;
		auto& _labelledArgumentPositionStorage = this->labelled_argument_positions_;


		// Determine parsed value offset/count into storage
		const auto _valueOffset = _parsedValuesStorage.size();
		const size_t _valueCount = _values.size();

		// Construct parsed argument info
		auto _parsedArgument = ParsedArgumentInfo
		{
			.offset = _valueOffset,
			.count = _valueCount
		};

		// Add values to parsed value storage.
		_parsedValuesStorage.insert(_parsedValuesStorage.end(), _values.begin(), _values.end());

		// Where the parsed argument info is being stored.
		const auto _parsedArgumentStorageIndex = _parsedArgumentStorage.size();

		// Add info to parsed argument storage
		_parsedArgumentStorage.push_back(_parsedArgument);

		// Add to labelled storage
		const auto [it, _noCollision] = _labelledArgumentPositionStorage.insert_or_assign
		(
			std::move(_label), _parsedArgumentStorageIndex
		);

		// Check for no name collision
		ASX_CHECK(_noCollision);
	};
};

namespace asx
{
	inline void assert_valid_option_name(std::string_view _testName, const std::string* _assertPretext = nullptr)
	{
		// Ensure this starts with '-' char(s)
		if (!_testName.starts_with('-'))
		{
			// Text never begins, for now this will be an invalid option name
			ASX_FAIL("Invalid option name \"{}\",{} must start with '-' or '--'",
				_testName,
				(_assertPretext) ? *_assertPretext : "");
		};

		// For now we will be extremely strict and stupid
		const auto _textStartPos = _testName.find_first_not_of('-');
		if (_textStartPos == _testName.npos)
		{
			// Text never begins, for now this will be an invalid option name
			ASX_FAIL("Invalid option name \"{}\",{} must contain text after '-' or '--'",
				_testName,
				(_assertPretext)? *_assertPretext : "");
		}
		else if (_textStartPos > 2)
		{
			// More than 2 '-' chars are present
			ASX_FAIL("Invalid option name \"{}\",{} initial characters must only be '-' or '--'",
				_testName,
				(_assertPretext)? *_assertPretext : "");
		};

		// If there is only one '-' character, the following text must be a single character long
		if (_textStartPos == 1 && _testName.size() > 2)
		{
			ASX_FAIL("Invalid option name \"{}\",{} options starting with '-' must be followed by only a single character",
				_testName,
				(_assertPretext)? *_assertPretext : "");
		};
	};


	ArgumentParser::ArgumentDefinition& ArgumentParser::ArgumentDefinitionHandle::get_definition() const
	{
		// Access definitions storage in parser.
		return this->parser_->argument_definitions_.at(this->index_);
	};
	
	ArgumentParser::ArgumentDefinitionHandle::self_type& ArgumentParser::ArgumentDefinitionHandle::add_name(std::string_view _name)
	{
		auto& _def = this->get_definition();

		// Check if name is an option (starts with "-" or "--")
		if (_name.starts_with('-'))
		{
			// Check valid option name
			assert_valid_option_name(_name);

			// Handle transition to an optional argument
			if (!_def.is_optional)
			{
				// Make sure all existing names are option names
				for (auto& v : _def.names)
				{
					std::string _pretext = asx::format(" existing name \"{}\" isn't valid -", v);
					assert_valid_option_name(v, &_pretext);
				};
			
				// Mark optional
				_def.is_optional = true;
			};
		}
		else if (_def.is_optional)
		{
			// Pass to valid option name check for error reporting.
			assert_valid_option_name(_name);
		};

		// Add to names
		_def.names.push_back(std::string(_name));
		return *this;
	};
};

namespace asx
{
	ArgumentParser::ParseResult ArgumentParser::parse_args_no_execute_filename(std::span<const std::string_view> _args)
	{
		// Resolve metalabels
		this->resolve_argument_metalabels();

		auto& _argumentDefinitions = this->argument_definitions_;



		// Container for referring to the positional arguments
		std::vector<const ArgumentDefinition*> _positionArgumentDefinitions;

		// Container for looking up arguments by name
		std::unordered_map<std::string, const ArgumentDefinition*> _argumentDefinitionNames;

		// Pointer to the help argument
		const ArgumentDefinition* _helpArgumentDefinition = nullptr;

		// Prepare helper data structures
		{
			const ArgumentDefinition* _foundOptionalPositional = nullptr;

			for (auto& _definition : _argumentDefinitions)
			{
				// Handle positional argument
				if (_definition.is_positional)
				{
					// If we already found an optional positional arg, this must be optional as well
					if (_foundOptionalPositional)
					{
						// Check if we found a non-optional positional after an optional
						// positional (this is illegal)
						if (!_definition.is_optional)
						{
							ASX_FAIL("Positional argument \"{}\" must be optional as it follows an optional positional argument \"{}\"",
								_definition.metalabel, _foundOptionalPositional->metalabel);
						};
					}
					// If this is the optional positional then we need to set the first tracker
					else if (_definition.is_optional)
					{
						_foundOptionalPositional = &_definition;
					};

					// Add to positional argument tracker
					_positionArgumentDefinitions.push_back(&_definition);
				}
				// Otherwise we have a named argument
				else
				{
					// Named arguments MUST be optional
					if (!_definition.is_optional)
					{
						ASX_FAIL("Named argument \"{}\" must be optional", _definition.label);
					};

					// Check if this is the help argument
					if (_definition.names.front() == "-h" || _definition.names.front() == "--help")
					{
						ASX_CHECK(!_helpArgumentDefinition);
						_helpArgumentDefinition = &_definition;
					};

					// Add to name map
					for (auto& _name : _definition.names)
					{
						const auto [_, _noCollision] =
							_argumentDefinitionNames.insert_or_assign(_name, &_definition);
						
						// Handle name collision error
						if (!_noCollision)
						{
							ASX_FAIL("Multiple arguments with the name \"{}\"", _name);
						};
					};
				};
			};
		};

		// Look for help option if we found an argument definition for it.
		if (_helpArgumentDefinition)
		{
			// Check args for matching name
			for (auto& _arg : _args)
			{
				if (jc::contains(_helpArgumentDefinition->names, _arg))
				{
					// Generate help text and exit early
					auto _helpText = this->generate_help_text();
					return ParseResult(true, false, std::move(_helpText));
				};
			}
		};

		auto _parseResult = ParseResult(false, false);


		return _parseResult;
	};
	ArgumentParser::ParseResult ArgumentParser::parse_args(std::span<const std::string_view> _args)
	{
		namespace fs = std::filesystem;

		// Check for executed filepath as first arg
		bool _firstArgIsExecutedFilepath = false;
		{
			// Get the true path to the executable (OS specific impl).
			const auto _currentExecutablePath = asx::get_current_executable_path();
		
			// Build and attempt resolve of path to running executable
			auto _possibleExecutablePath = fs::absolute(_args.front());
			std::error_code _errc{};
			_possibleExecutablePath = fs::weakly_canonical(_possibleExecutablePath, _errc);

			// If we were able to make the path canonical and it matches the current executable's path
			// then we can assume this is the path to the running executable.
			if (!_errc && _possibleExecutablePath == _currentExecutablePath)
			{
				// First arg resolved to executable path so therefore we assume it is executed filename.
				_firstArgIsExecutedFilepath = true;
			};
		};

		// If first arg is executed file path, remove it from the given args
		if (_firstArgIsExecutedFilepath)
		{
			_args = _args.last(_args.size() - 1);
		};

		// Parse with executed filepath now removed.
		return this->parse_args_no_execute_filename(_args);
	};
	ArgumentParser::ParseResult ArgumentParser::parse_args(int _nargs, const char* const* _vargs)
	{
		// Construct storage for _vargs as string views
		auto _vargStrings = std::vector<std::string_view>(&_vargs[0], &_vargs[_nargs]);

		// Invoke root parse_args function
		return this->parse_args(_vargStrings);
	};


	ArgumentParser::ArgumentDefinitionHandle ArgumentParser::add_argument(std::string_view _label, std::string_view _description)
	{
		// Grab where the new definition will be
		const auto _index = this->argument_definitions_.size();

		// Add new definition
		auto _definition = ArgumentDefinition{};
		_definition.label = _label;
		_definition.description = _description;
		this->argument_definitions_.push_back(std::move(_definition));

		// Return handle to new definition
		return ArgumentDefinitionHandle(*this, _index);
	};


	ArgumentParser::ArgumentParser(const std::string& _name, const std::string& _description) :
		name_(_name), description_(_description)
	{
		// Always define default args
		this->define_default_arguments();
	};
	ArgumentParser::ArgumentParser(const std::string& _name) :
		ArgumentParser(_name, std::string{})
	{};


	void ArgumentParser::define_default_arguments()
	{
		// Define -h and --help arguments

		this->add_argument("help", "Displays the help message")
			.add_name("-h")
			.add_name("--help");

	};

	void ArgumentParser::resolve_argument_metalabels()
	{
		size_t _positionalArgumentCounter = 0;

		for (auto& _definition : this->argument_definitions_)
		{
			// Resolve metalabel text if it hasn't been specified.
			if (_definition.metalabel.empty())
			{
				// Use names if any were specified
				if (!_definition.names.empty())
				{
					// Use names
					std::string _nameLabelText{};

					// Concat like "names..." with '|' as seperators
					bool _firstName = true;
					for (auto& _name : _definition.names)
					{
						// Add seperator after first name
						if (!_firstName)
						{
							_nameLabelText.push_back('|');
						};

						// Add name
						_nameLabelText.append(_name);

						// Ensure we only skip the seperator for the first name
						_firstName = false;
					};

					// Set metalabel
					_definition.metalabel = std::move(_nameLabelText);
				}
				else if (!_definition.label.empty())
				{
					// Fall back to label text
					_definition.metalabel = _definition.label;
				}
				else
				{
					// Use argument position
					ASX_CHECK(_definition.is_positional);
					_definition.metalabel = asx::format("arg{}", _positionalArgumentCounter);
				};
			};

			// Increment positional if needed
			if (_definition.is_positional)
			{
				++_positionalArgumentCounter;
			};
		};
	};

	std::string ArgumentParser::generate_help_text()
	{
		const auto& _argumentDefinitions = this->argument_definitions_;

		// Stream to write text into, this can be improved but really isn't performance
		// critical so I think its a non-issue.
		std::ostringstream _sstr{};

		// Write usage text
		_sstr << "Usage:\n\t" << this->name_ << ' ';
		
		// Add argument metalabel text
		for (auto& _definition : _argumentDefinitions)
		{
			if (_definition.is_optional)
			{
				_sstr << '[' << _definition.metalabel << ']';
			}
			else
			{
				_sstr << '<' << _definition.metalabel << '>';
			};
		};


		return _sstr.str();
	};
};
