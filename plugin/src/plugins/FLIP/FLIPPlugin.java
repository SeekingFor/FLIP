package plugins.FLIP;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;

import freenet.pluginmanager.FredPlugin;
import freenet.pluginmanager.FredPluginRealVersioned;
import freenet.pluginmanager.FredPluginThreadless;
import freenet.pluginmanager.FredPluginVersioned;
import freenet.pluginmanager.PluginRespirator;
import freenet.support.io.Closer;

public class FLIPPlugin implements FredPlugin, FredPluginRealVersioned, FredPluginThreadless, FredPluginVersioned {
	/** Shut down the plugin. */
	@Override
	public void terminate()
	{
		StopFLIP();
	}

	/** Run the plugin. Called after node startup. Should be able to access
	 * queue etc at this point. Plugins which do not implement
	 * FredPluginThreadless will be terminated after they return from this
	 * function. Threadless plugins will not terminate until they are
	 * explicitly unloaded. */
	@Override
	public void runPlugin(PluginRespirator pr)
	{
		StartFLIP();
	}

	@Override
	public long getRealVersion()
	{
		return VersionLong();
	}

	@Override
	public String getVersion()
	{

		return VersionString();
	}

	// JNI methods
	public native void StartFLIP();
	public native void StopFLIP();
	public native String VersionString();
	public native long VersionLong();

	private static final boolean tryLoadResource(File f, URL resource)
		throws FileNotFoundException, UnsatisfiedLinkError {
		InputStream is;
		try {
			is = resource.openStream();
		} catch(IOException e) {
			f.delete();
			throw new FileNotFoundException();
		}

		FileOutputStream fos = null;
		try {
			f.deleteOnExit();
			fos = new FileOutputStream(f);
			byte[] buf = new byte[4096 * 1024];
			int read;
			while((read = is.read(buf)) > 0) {
				fos.write(buf, 0, read);
			}
			fos.close();
			fos = null;
			System.load(f.getAbsolutePath());
			return true;
		} catch(IOException e) {
		} catch(UnsatisfiedLinkError ule) {
			// likely to be "noexec"
			if(ule.toString().toLowerCase().indexOf("not permitted") == -1)
				throw ule;
		} finally {
			Closer.close(fos);
			f.delete();
		}

		return false;
	}

	static
	{
		String resourcepath=FLIPPlugin.class.getPackage().getName().replace('.', '/')+"/FLIP-"+getPlatform()+"-"+getArchitechture()+"."+getSuffix();
		URL resource=FLIPPlugin.class.getClassLoader().getResource(resourcepath);
		File temp=null;

		try
		{
			temp=File.createTempFile("FLIP","bin.tmp");
			tryLoadResource(temp,resource);
		}
		catch(IOException e)
		{
		}
		finally
		{
			if(temp!=null)
			{
				temp.delete();
			}
		}
	}

	private static String getPlatform()
	{
		boolean isWindows = (System.getProperty("os.name").toLowerCase().indexOf("windows") != -1);
		boolean isLinux = (System.getProperty("os.name").toLowerCase().indexOf("linux") != -1);
		boolean isFreebsd = (System.getProperty("os.name").toLowerCase().indexOf("freebsd") != -1);
		boolean isMacOS = (System.getProperty("os.name").toLowerCase().indexOf("mac os x") != -1);
		if(isWindows)
			return "win"; // The convention on Windows
		if(isLinux)
			return "linux"; // The convention on linux...
		if(isFreebsd)
			return "freebsd"; // The convention on freebsd...
		if(isMacOS)
			return "osx"; // The convention on Mac OS X...
		throw new RuntimeException("Dont know library name for os type '" + System.getProperty("os.name") + '\'');
	}

	private static String getArchitechture()
	{
		String arch;
		if(System.getProperty("os.arch").toLowerCase().matches("(i?[x0-9]86_64|amd64)")) {
			arch = "amd64";
		} else if(System.getProperty("os.arch").toLowerCase().matches("(ppc)")) {
			arch = "ppc";
		} else {
			// We want to try the i386 libraries if the architecture is unknown
			arch = "i386";
		}
		return arch;
	}

	private static String getSuffix()
	{
		boolean isWindows = System.getProperty("os.name").toLowerCase().indexOf("windows") != -1;
		boolean isMacOS = (System.getProperty("os.name").toLowerCase().indexOf("mac os x") != -1);
		if(isWindows)
			return "dll";
		else if(isMacOS)
			return "jnilib";
		else
			return "so";
	}

}
